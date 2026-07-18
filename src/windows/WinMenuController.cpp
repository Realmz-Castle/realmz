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

#include "../PortMenu.hpp"
#include "./WinMenuController.hpp"
#include <utility>

static phosg::PrefixedLogger wmc_log("[WinMenuController] ");
static constexpr char kPopupDiamondMark = 19;
static constexpr size_t kPopupMarkGutterWidth = 12;
static constexpr int kPopupMarkRadius = 3;
static constexpr uint32_t kPopupMarkPixel = 0xFF000000;
static constexpr uint32_t kPopupMixedMarkPixel = 0xFF808080;

enum class PopupMenuMark {
  None,
  Checked,
  Mixed,
};

// QuickDraw.hpp declares Mac QuickDraw APIs whose names collide with Win32 headers here.
phosg::ImageRGBA8888N DecodeCIconImage(int16_t iconID);

// Command IDs for the Port menu. They sit in a reserved high range so they never
// collide with the packed (menu_id, item_id) identifiers the game's own menus use,
// which top out at 0x7F7F because menu_id and item_id are each a single byte.
static constexpr WORD PORT_CMD_BASE = 0xE000;
static constexpr WORD PORT_CMD_MIN = PORT_CMD_BASE;
static constexpr WORD PORT_CMD_MAX = 0xE0FF;

static bool IsPortCommand(WORD cmd) {
  return (cmd >= PORT_CMD_MIN) && (cmd <= PORT_CMD_MAX);
}

// Builds the Port menu and appends it to the given menu bar. Mirrors the layout of
// the macOS Port menu (filters, a Scale submenu, an aspect-lock toggle, and a
// Color Correction submenu).
static void BuildPortMenu(HMENU menubar) {
  HMENU port_menu = CreatePopupMenu();

  HMENU filter_menu = CreatePopupMenu();
  for (int i = 0; i < kPortFilterCount; i++) {
    AppendMenu(filter_menu, MF_STRING, PORT_CMD_BASE + kPortFilterId + i, kPortFilters[i].title);
  }
  AppendMenu(port_menu, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>(filter_menu), "Filter");

  HMENU scale_menu = CreatePopupMenu();
  for (int i = 0; i < kPortScaleCount; i++) {
    AppendMenu(scale_menu, MF_STRING, PORT_CMD_BASE + kPortScaleId + i, kPortScales[i].title);
  }
  AppendMenu(port_menu, MF_POPUP, reinterpret_cast<UINT_PTR>(scale_menu), "Scale");
  AppendMenu(port_menu, MF_STRING, PORT_CMD_BASE + kPortAspectLockId, "Lock Aspect Ratio");

  HMENU gamma_menu = CreatePopupMenu();
  for (int i = 0; i < kPortGammaCount; i++) {
    AppendMenu(gamma_menu, MF_STRING, PORT_CMD_BASE + kPortGammaId + i, kPortGammaOptions[i].title);
  }

  AppendMenu(port_menu, MF_SEPARATOR, 0, nullptr);
  AppendMenu(port_menu, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>(gamma_menu), "Color Correction");

  AppendMenu(menubar, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>(port_menu), "Port");
}

// Refreshes the checkmarks and enabled state of the Port menu items in the given
// popup. Called from WM_INITMENUPOPUP so the state is current each time the menu (or
// its Scale submenu) opens, the same role the menuNeedsUpdate delegate plays on macOS.
static void UpdatePortMenuState(HMENU menu) {
  if (!menu) {
    return;
  }
  int count = GetMenuItemCount(menu);
  for (int pos = 0; pos < count; pos++) {
    UINT cmd = GetMenuItemID(menu, pos);
    if (!IsPortCommand(static_cast<WORD>(cmd))) {
      continue;
    }
    int checked = 0, enabled = 1;
    PortMenu_ItemState(cmd - PORT_CMD_BASE, &checked, &enabled);
    EnableMenuItem(menu, pos, MF_BYPOSITION | (enabled ? MF_ENABLED : MF_GRAYED));
    CheckMenuItem(menu, pos, MF_BYPOSITION | (checked ? MF_CHECKED : MF_UNCHECKED));
  }
}

static void HandlePortCommand(WORD cmd) {
  PortMenu_Apply(cmd - PORT_CMD_BASE);
}

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

static void DrawPopupMenuMark(
    uint32_t* pixels,
    size_t stride,
    size_t mark_width,
    size_t height,
    PopupMenuMark mark,
    uint32_t mark_pixel = kPopupMarkPixel) {
  if ((mark == PopupMenuMark::None) ||
      (mark_width < (kPopupMarkRadius * 2 + 1)) ||
      (height < (kPopupMarkRadius * 2 + 1))) {
    return;
  }

  int center_x = static_cast<int>(mark_width / 2);
  int center_y = static_cast<int>(height / 2);
  if (mark == PopupMenuMark::Mixed) {
    for (int dx = -kPopupMarkRadius; dx <= kPopupMarkRadius; dx++) {
      pixels[(center_y * stride) + center_x + dx] = mark_pixel;
    }
    return;
  }

  for (int dy = -kPopupMarkRadius; dy <= kPopupMarkRadius; dy++) {
    int span = kPopupMarkRadius - ((dy < 0) ? -dy : dy);
    for (int dx = -span; dx <= span; dx++) {
      pixels[((center_y + dy) * stride) + center_x + dx] = mark_pixel;
    }
  }
}

static HBITMAP CreateBitmapForMenuIcon(int16_t icon_id, PopupMenuMark mark) {
  if (icon_id <= 0) {
    return NULL;
  }

  try {
    auto image = DecodeCIconImage(icon_id);
    auto icon_width = image.get_width();
    auto height = image.get_height();
    if (!icon_width || !height) {
      return NULL;
    }
    auto width = icon_width + kPopupMarkGutterWidth;

    uint32_t* dst = nullptr;
    HBITMAP bitmap = CreateMenuBitmap(width, height, &dst);
    if (!bitmap) {
      return NULL;
    }

    const uint32_t* src = image.get_data();
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < icon_width; x++) {
        dst[(y * width) + kPopupMarkGutterWidth + x] =
            BgraForRgba(src[(y * icon_width) + x]);
      }
    }

    DrawPopupMenuMark(dst, width, kPopupMarkGutterWidth, height, mark);

    return bitmap;
  } catch (const std::exception& e) {
    wmc_log.warning_f("Could not create menu icon {}: {}", icon_id, e.what());
    return NULL;
  }
}

static PopupMenuMark PopupMenuMarkForItem(const WinMenu::Item& item) {
  if (item.checked || item.mark_character == kPopupDiamondMark) {
    return PopupMenuMark::Checked;
  }
  return item.mark_character ? PopupMenuMark::Mixed : PopupMenuMark::None;
}

static HBITMAP CreateMixedMenuMarkBitmap() {
  size_t width = GetSystemMetrics(SM_CXMENUCHECK);
  size_t height = GetSystemMetrics(SM_CYMENUCHECK);
  uint32_t* pixels = nullptr;
  HBITMAP bitmap = CreateMenuBitmap(width, height, &pixels);
  if (bitmap) {
    DrawPopupMenuMark(pixels, width, width, height, PopupMenuMark::Mixed, kPopupMixedMarkPixel);
  }
  return bitmap;
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
  if (msg == WM_INITMENUPOPUP) {
    UpdatePortMenuState(reinterpret_cast<HMENU>(wParam));
    return CallWindowProc(g_OldWndProc, hwnd, msg, wParam, lParam);
  }

  if (msg == WM_COMMAND) {
    WORD cmd = LOWORD(wParam);
    if (IsPortCommand(cmd)) {
      HandlePortCommand(cmd);
      return 0;
    }
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

// Classic Mac marks a menu item as the parent of a hierarchical submenu by setting its command
// char (key_equivalent here) to 0x1B; the mark_character then holds the submenu's menu id.
// Macintosh Toolbox Essentials (3-96).
static constexpr char hMenuCmd = 0x1B;

// Builds the Windows drop-down/submenu for a Realmz menu, recursing into any hierarchical
// submenus (such as Volume and Speed) referenced via the 0x1B/mark_character marker.
static HMENU BuildMenu(const std::shared_ptr<WinMenu>& menu, const WinMenuList& menu_list) {
  HMENU win_submenu = CreateMenu();

  uint16_t i = 1;
  for (const auto& submenu_item : menu->items) {
    UINT enabled_state = submenu_item.enabled ? MFS_ENABLED : MFS_DISABLED;

    bool is_hierarchical = (submenu_item.key_equivalent == hMenuCmd) && submenu_item.mark_character;

    // A non-submenu item carries its mark in mark_character (set via SetItemMark): 19 is the
    // diamond "selected" glyph, -41 a secondary state. Native Windows menus have no "mixed"
    // state, so collapse every nonzero mark to a check. Submenu parents store the child menu id
    // in mark_character, so key_equivalent == 0x1B guards against treating that as a mark.
    bool marked = submenu_item.checked || (!is_hierarchical && submenu_item.mark_character != 0);
    UINT checked_state = marked ? MFS_CHECKED : MFS_UNCHECKED;

    std::string name = submenu_item.name;
    if (submenu_item.key_equivalent && !is_hierarchical) {
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

    // Attach the referenced submenu, if this item is a hierarchical-menu parent.
    if (is_hierarchical) {
      for (const auto& sub : menu_list.submenus) {
        if (sub->menu_id == static_cast<uint8_t>(submenu_item.mark_character)) {
          submenu_info.fMask |= MIIM_SUBMENU;
          submenu_info.hSubMenu = BuildMenu(sub, menu_list);
          break;
        }
      }
    }

    InsertMenuItem(win_submenu, i++, TRUE, &submenu_info);
  }

  return win_submenu;
}

void WinMenuSync(SDL_Window* sdl_window, std::shared_ptr<WinMenuList> menu_list, void (*callback)(int16_t, int16_t)) {
  menuCallback = callback;
  current_menu_list = menu_list;

  auto wind_handle = get_window_handle(sdl_window);

  // Capture the current logical size so it can be reapplied after the menu bar is
  // attached (see the note below). Reapplying the current size rather than the fixed
  // logical default keeps a scale chosen from the Port menu from being reset on the next sync.
  int client_w = kLogicalWindowWidth, client_h = kLogicalWindowHeight;
  SDL_GetWindowSize(sdl_window, &client_w, &client_h);

  HMENU win_menu = CreateMenu();
  MENUINFO win_menu_info = MENUINFO{
      .cbSize = sizeof(MENUINFO),
      .fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE};
  SetMenuInfo(win_menu, &win_menu_info);

  for (auto menu : menu_list->menus) {
    auto submenu = BuildMenu(menu, *menu_list);

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

  BuildPortMenu(win_menu);

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
  // enough to force SDL to realize the menu bar now exists and to expand the window so the client area
  // is the full size it expects. Reapply the size captured above rather than the fixed logical
  // default so a scale picked from the Port menu survives a menu re-sync.
  SDL_SetWindowSize(sdl_window, client_w, client_h);
}

int WinCreatePopupMenu(SDL_Window* sdl_window, std::shared_ptr<WinMenu> menu, int window_x, int window_y) {
  auto wind_handle = get_window_handle(sdl_window);

  HMENU popupMenu = CreatePopupMenu();

  std::vector<HBITMAP> owned_bitmaps;
  HBITMAP mixed_bitmap = NULL;

  int i{0};
  for (const auto& item : menu->items) {
    i++;
    PopupMenuMark mark = PopupMenuMarkForItem(item);
    HBITMAP item_bitmap = CreateBitmapForMenuIcon(item.icon_id, mark);
    if (item_bitmap) {
      owned_bitmaps.emplace_back(item_bitmap);
    } else if (!mixed_bitmap && (mark == PopupMenuMark::Mixed)) {
      mixed_bitmap = CreateMixedMenuMarkBitmap();
      if (mixed_bitmap) {
        owned_bitmaps.emplace_back(mixed_bitmap);
      }
    }

    // Windows does not reserve a native check gutter beside hbmpItem. Icon rows
    // therefore carry their marks in the bitmap; text-only rows use the check column.
    UINT flags = MF_STRING |
        (item.enabled ? MF_ENABLED : MF_GRAYED) |
        ((mark == PopupMenuMark::None) ? MF_UNCHECKED : MF_CHECKED);
    AppendMenu(popupMenu, flags, static_cast<UINT_PTR>(i), item.name.c_str());

    MENUITEMINFO item_info = MENUITEMINFO{.cbSize = sizeof(MENUITEMINFO)};
    if (!item_bitmap && mixed_bitmap && (mark == PopupMenuMark::Mixed)) {
      item_info.fMask |= MIIM_CHECKMARKS;
      item_info.hbmpChecked = mixed_bitmap;
    }
    if (item_bitmap) {
      item_info.fMask |= MIIM_BITMAP;
      item_info.hbmpItem = item_bitmap;
    }
    if (item_info.fMask) {
      SetMenuItemInfo(popupMenu, static_cast<UINT>(i), FALSE, &item_info);
    }
  }

  // TrackPopupMenu displays the menu in screen coordinates. The caller hands us the requested
  // position in window (client-area) coordinates, already converted from the game's logical
  // render space, so convert client -> screen here to anchor the menu where the caller asked
  // rather than wherever the cursor happens to be.
  POINT pt{window_x, window_y};
  ClientToScreen(wind_handle, &pt);

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
