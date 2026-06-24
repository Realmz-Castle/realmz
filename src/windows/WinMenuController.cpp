#include "errhandlingapi.h"
#include "windef.h"
#include "wingdi.h"
#include "winuser.h"
#include <algorithm>
#include <cstdint>
#include <exception>
#include <memory>
#include <phosg/Image.hh>
#include <phosg/Strings.hh>
#include <vector>

#include "./WinMenuController.hpp"
#include <utility>

static phosg::PrefixedLogger wmc_log("[WinMenuController] ");
static constexpr char kPopupDiamondMark = 19;
static constexpr size_t kPopupMarkBitmapSize = 13;
static constexpr int kPopupMarkRadius = 3;
static constexpr uint32_t kOpaqueBlackPixel = 0xFF000000;

// QuickDraw.hpp declares Mac QuickDraw APIs whose names collide with Win32 headers here.
phosg::ImageRGBA8888N DecodeCIconImage(int16_t iconID);

// Static variable to keep the original window proc
static WNDPROC g_OldWndProc = nullptr;

// Callback to invoke with clicked menu items. Should be a pointer to a function that
// accepts two int16_t params, the menu_id and the item_id (which is the 1-indexed position of
// the item in the menu)
static void (*menuCallback)(int16_t, int16_t){};

// Current menu list for keyboard shortcut lookup
static std::shared_ptr<WinMenuList> current_menu_list;

// Packs the menu id and item id of each submenu item into a single word. When a command menu
// item is clicked, Windows sends a WM_COMMAND message with the low byte of the wParam filled
// with the wID property of the MENUITEMINFO struct of the menu. By packing both the Realmz
// menu_id and the position of the submenu as the item_id, we can extract these values when
// handling WM_COMMAND messages and convert them into synthetic menu click events to send
// to the Realmz event loop.
WORD PackMenuIdentifier(int8_t menu_id, int8_t item_id) {
  return (menu_id << 8) | item_id;
}

// Returns a pair with the menu_id and item_id from a packed wParam
std::pair<int16_t, int16_t> UnpackMenuIdentifier(WORD wParam) {
  return {(wParam >> 8) & 0x00FF, wParam & 0x00FF};
}

static HBITMAP CreateMenuBitmap(size_t width, size_t height, uint32_t** pixels_out) {
  BITMAPINFO bitmap_info = BITMAPINFO{
      .bmiHeader = {
          .biSize = sizeof(BITMAPINFOHEADER),
          .biWidth = static_cast<LONG>(width),
          .biHeight = -static_cast<LONG>(height),
          .biPlanes = 1,
          .biBitCount = 32,
          .biCompression = BI_RGB}};

  void* pixels = nullptr;
  HBITMAP bitmap = CreateDIBSection(NULL, &bitmap_info, DIB_RGB_COLORS, &pixels, NULL, 0);
  if (!bitmap || !pixels) {
    if (bitmap) {
      DeleteObject(bitmap);
    }
    return NULL;
  }

  *pixels_out = static_cast<uint32_t*>(pixels);
  std::fill(*pixels_out, *pixels_out + (width * height), 0);
  return bitmap;
}

static uint32_t BgraForRgba(uint32_t rgba) {
  uint8_t r = static_cast<uint8_t>(rgba >> 24);
  uint8_t g = static_cast<uint8_t>(rgba >> 16);
  uint8_t b = static_cast<uint8_t>(rgba >> 8);
  uint8_t a = static_cast<uint8_t>(rgba);
  return (
      (static_cast<uint32_t>(a) << 24) |
      (static_cast<uint32_t>(r) << 16) |
      (static_cast<uint32_t>(g) << 8) |
      static_cast<uint32_t>(b));
}

static HBITMAP CreateBitmapForMenuIcon(int16_t icon_id) {
  if (icon_id <= 0) {
    return NULL;
  }

  try {
    auto image = DecodeCIconImage(icon_id);
    auto width = image.get_width();
    auto height = image.get_height();
    if (!width || !height) {
      return NULL;
    }

    uint32_t* dst = nullptr;
    HBITMAP bitmap = CreateMenuBitmap(width, height, &dst);
    if (!bitmap) {
      return NULL;
    }

    const uint32_t* src = image.get_data();
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++) {
        dst[(y * width) + x] = BgraForRgba(src[(y * width) + x]);
      }
    }

    return bitmap;
  } catch (const std::exception& e) {
    wmc_log.warning_f("Could not create menu icon {}: {}", icon_id, e.what());
    return NULL;
  }
}

static HBITMAP CreatePopupMenuMarkBitmap(bool draw_mark) {
  constexpr size_t center_x = kPopupMarkBitmapSize / 2;
  constexpr size_t center_y = kPopupMarkBitmapSize / 2;

  uint32_t* pixels = nullptr;
  HBITMAP bitmap = CreateMenuBitmap(kPopupMarkBitmapSize, kPopupMarkBitmapSize, &pixels);
  if (!bitmap) {
    return NULL;
  }

  if (!draw_mark) {
    return bitmap;
  }

  for (int dy = -kPopupMarkRadius; dy <= kPopupMarkRadius; dy++) {
    int span = kPopupMarkRadius - ((dy < 0) ? -dy : dy);
    for (int dx = -span; dx <= span; dx++) {
      size_t x = static_cast<size_t>(static_cast<int>(center_x) + dx);
      size_t y = static_cast<size_t>(static_cast<int>(center_y) + dy);
      pixels[(y * kPopupMarkBitmapSize) + x] = kOpaqueBlackPixel;
    }
  }

  return bitmap;
}

static bool IsPopupMenuItemMarked(const WinMenu::Item& item) {
  return item.checked || item.mark_character == kPopupDiamondMark;
}

// Returns {menu_id, item_id}, or {0, 0} if not found
std::pair<int16_t, int16_t> FindMenuItemByKeyEquivalent(char ch) {
  wmc_log.info_f("Looking for menu item with key {:c} ({:02X})", ch, ch);
  if (!current_menu_list) {
    wmc_log.info_f("No menus are loaded");
    return {0, 0};
  }

  ch = toupper(ch);
  for (const auto& menu_set : {current_menu_list->menus, current_menu_list->submenus}) {
    for (const auto& menu : menu_set) {
      wmc_log.info_f("Looking in menu \"{}\"", menu->title);
      if (!menu->enabled) {
        continue;
      }
      for (size_t z = 0; z < menu->items.size(); z++) {
        const auto& item = menu->items[z];
        wmc_log.info_f("Looking at item \"{}\" -> \"{}\" ({:02X})", menu->title, item.name, item.key_equivalent, ch);
        if (item.enabled && toupper(item.key_equivalent) == ch) {
          wmc_log.info_f("Found menu item ID ({}, {})", menu->menu_id, z);
          return {menu->menu_id, z + 1};
        }
      }
    }
  }

  wmc_log.info_f("No menu item matched the given key");
  return {0, 0};
}

LRESULT CALLBACK RealmzWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_COMMAND) {
    if (menuCallback != nullptr) {
      auto identifier_pair = UnpackMenuIdentifier(wParam);
      menuCallback(identifier_pair.first, identifier_pair.second);
    }
    return 0;
  }

  if ((msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN)) {
    wmc_log.info_f("WM_(SYS)?KEYDOWN: wParam = {:04X}, menuCallback present = {}", wParam, (menuCallback != nullptr));
  }
  if (((msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN)) && (menuCallback != nullptr) && (GetKeyState(VK_CONTROL) & 0x8000)) {
    char ch = static_cast<char>(wParam);

    auto menu_item = FindMenuItemByKeyEquivalent(ch);
    if (menu_item.first != 0) {
      wmc_log.info_f("Received menu keyboard shortcut: Ctrl+{} -> menu={}, item={}",
          ch, menu_item.first, menu_item.second);
      menuCallback(menu_item.first, menu_item.second);
      return 0;
    }
  }

  // Forward everything else to the original WndProc
  return CallWindowProc(g_OldWndProc, hwnd, msg, wParam, lParam);
}

void HookWndProc(HWND hwnd) {
  if (g_OldWndProc == nullptr) {
    SetLastError(0);
    g_OldWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)RealmzWndProc);
    if (g_OldWndProc == nullptr) {
      wmc_log.error_f("Could not hook custom proc: %s", GetLastError());
    }
  }
}

HWND get_window_handle(SDL_Window* sdl_window) {
  auto props = SDL_GetWindowProperties(sdl_window);
  return reinterpret_cast<HWND>(SDL_GetPointerProperty(
      props,
      SDL_PROP_WINDOW_WIN32_HWND_POINTER,
      NULL));
}

void WinMenuSync(SDL_Window* sdl_window, std::shared_ptr<WinMenuList> menu_list, void (*callback)(int16_t, int16_t)) {
  // Update current menu click callback function
  menuCallback = callback;

  // Store the current menu list for keyboard shortcut lookup
  current_menu_list = menu_list;

  auto wind_handle = get_window_handle(sdl_window);

  HMENU win_menu = CreateMenu();
  MENUINFO win_menu_info = MENUINFO{
      .cbSize = sizeof(MENUINFO),
      .fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE};
  SetMenuInfo(win_menu, &win_menu_info);

  uint16_t i;
  for (auto menu : menu_list->menus) {
    auto submenu = CreateMenu();

    i = 1;
    for (const auto& submenu_item : menu->items) {
      UINT enabled_state = submenu_item.enabled ? MFS_ENABLED : MFS_DISABLED;
      UINT checked_state = submenu_item.checked ? MFS_CHECKED : MFS_UNCHECKED;

      std::string name = submenu_item.name;
      if (submenu_item.key_equivalent) {
        name += std::format("\tCtrl+{:c}", toupper(submenu_item.key_equivalent));
      }
      MENUITEMINFO submenu_info = MENUITEMINFO{
          .cbSize = sizeof(MENUITEMINFO),
          .fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING,
          .fType = MFT_STRING,
          .fState = enabled_state | checked_state,
          .wID = PackMenuIdentifier(menu->menu_id, i),
          .dwTypeData = const_cast<char*>(name.c_str()),
          .cch = static_cast<UINT>(name.length())};

      InsertMenuItem(submenu, i++, TRUE, &submenu_info);
    }

    MENUITEMINFO item_info = MENUITEMINFO{
        .cbSize = sizeof(MENUITEMINFO),
        .fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU,
        .fType = MFT_STRING,
        .fState = static_cast<UINT>(menu->enabled ? MFS_ENABLED : MFS_DISABLED),
        .wID = static_cast<UINT>(menu->menu_id),
        .hSubMenu = submenu,
        .hbmpChecked = NULL,
        .hbmpUnchecked = NULL,
        .dwItemData = NULL,
        .dwTypeData = const_cast<char*>(menu->title.c_str()),
        .cch = static_cast<UINT>(menu->title.length()),
        .hbmpItem = NULL};
    InsertMenuItem(win_menu, menu->menu_id, FALSE, &item_info);
  }

  auto old_menu = GetMenu(wind_handle);
  SetMenu(wind_handle, win_menu);
  HookWndProc(wind_handle);

  DrawMenuBar(wind_handle);

  if (old_menu) {
    DestroyMenu(old_menu);
  }

  // After experimenting with this, it seems that calls to SDL_SetWindowSize actually set the
  // client area of the window, not the full window size inclusive of the menu bar. Since we have to
  // bypass SDL to create the menu directly via the Windows API, it seems that SDL doesn't know that
  // the rendering of the menu bar has shrunk the client area. So, a quick call to SDL_SetWindowSize is
  // enough to force SDL to realize the menu bar now exists and to expand the window to ensure that the
  // client area is the full 800x600.
  SDL_SetWindowSize(sdl_window, 800, 600);
}

int WinCreatePopupMenu(SDL_Window* sdl_window, std::shared_ptr<WinMenu> menu) {
  auto wind_handle = get_window_handle(sdl_window);

  HMENU popupMenu = CreatePopupMenu();

  bool has_marked_item = false;
  for (const auto& item : menu->items) {
    if (IsPopupMenuItemMarked(item)) {
      has_marked_item = true;
      break;
    }
  }

  std::vector<HBITMAP> owned_bitmaps;
  // The blank unchecked bitmap keeps icon columns aligned when only some rows are marked.
  HBITMAP checked_bitmap = has_marked_item ? CreatePopupMenuMarkBitmap(true) : NULL;
  HBITMAP unchecked_bitmap = has_marked_item ? CreatePopupMenuMarkBitmap(false) : NULL;
  if (checked_bitmap) {
    owned_bitmaps.emplace_back(checked_bitmap);
  }
  if (unchecked_bitmap) {
    owned_bitmaps.emplace_back(unchecked_bitmap);
  }

  int i{0};
  for (const auto& item : menu->items) {
    i++;
    bool marked = IsPopupMenuItemMarked(item);
    HBITMAP item_bitmap = CreateBitmapForMenuIcon(item.icon_id);
    if (item_bitmap) {
      owned_bitmaps.emplace_back(item_bitmap);
    }

    UINT enabled_state = item.enabled ? MFS_ENABLED : MFS_GRAYED;
    MENUITEMINFO item_info = MENUITEMINFO{
        .cbSize = sizeof(MENUITEMINFO),
        .fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING,
        .fType = MFT_STRING,
        .fState = enabled_state | (marked ? MFS_CHECKED : MFS_UNCHECKED),
        .wID = static_cast<UINT>(i),
        .dwTypeData = const_cast<char*>(item.name.c_str()),
        .cch = static_cast<UINT>(item.name.length())};
    if (checked_bitmap && unchecked_bitmap) {
      item_info.fMask |= MIIM_CHECKMARKS;
      item_info.hbmpChecked = checked_bitmap;
      item_info.hbmpUnchecked = unchecked_bitmap;
    }
    if (item_bitmap) {
      item_info.fMask |= MIIM_BITMAP;
      item_info.hbmpItem = item_bitmap;
    }

    InsertMenuItem(popupMenu, i - 1, TRUE, &item_info);
  }

  // TrackPopupMenu displays the menu in screen coordinates, not window coordinates. Rather
  // thank require the caller to convert the mouse position from local to global coordinates,
  // it's easier to just get the mouse position fresh right here.
  POINT pt;
  GetCursorPos(&pt);

  int result = TrackPopupMenu(popupMenu,
      TPM_RETURNCMD | TPM_RIGHTBUTTON,
      pt.x, pt.y,
      0,
      wind_handle,
      NULL);

  DestroyMenu(popupMenu);
  for (HBITMAP bitmap : owned_bitmaps) {
    DeleteObject(bitmap);
  }

  return result;
}
