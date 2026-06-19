#include "errhandlingapi.h"
#include "windef.h"
#include "winuser.h"
#include <memory>
#include <phosg/Strings.hh>

#include "../PortMenu.hpp"
#include "./WinMenuController.hpp"
#include <utility>

static phosg::PrefixedLogger wmc_log("[WinMenuController] ");

// Command IDs for the Port menu. They sit in a reserved high range so they never
// collide with the packed (menu_id, item_id) identifiers the game's own menus use,
// which top out at 0x7F7F because menu_id and item_id are each a single byte.
static constexpr WORD PORT_FILTER_BASE = 0xE000; // + filter index
static constexpr WORD PORT_SCALE_BASE = 0xE010; // + scale index
static constexpr WORD PORT_ASPECT_LOCK = 0xE020;
static constexpr WORD PORT_GAMMA_BASE = 0xE030; // + gamma index
static constexpr WORD PORT_CMD_MIN = PORT_FILTER_BASE;
static constexpr WORD PORT_CMD_MAX = 0xE0FF;

static bool IsPortCommand(WORD cmd) {
  return (cmd >= PORT_CMD_MIN) && (cmd <= PORT_CMD_MAX);
}

static bool PortCommandKindIndex(WORD cmd, PortCmdKind* kind, int* index) {
  if ((cmd >= PORT_FILTER_BASE) && (cmd < PORT_FILTER_BASE + kPortFilterCount)) {
    *kind = PortCmdFilter;
    *index = cmd - PORT_FILTER_BASE;
  } else if ((cmd >= PORT_SCALE_BASE) && (cmd < PORT_SCALE_BASE + kPortScaleCount)) {
    *kind = PortCmdScale;
    *index = cmd - PORT_SCALE_BASE;
  } else if (cmd == PORT_ASPECT_LOCK) {
    *kind = PortCmdAspectLock;
    *index = 0;
  } else if ((cmd >= PORT_GAMMA_BASE) && (cmd < PORT_GAMMA_BASE + kPortGammaCount)) {
    *kind = PortCmdGamma;
    *index = cmd - PORT_GAMMA_BASE;
  } else {
    return false;
  }
  return true;
}

// Builds the Port menu and appends it to the given menu bar. Mirrors the layout of
// the macOS Port menu (filters, a Scale submenu, an aspect-lock toggle, and a
// disabled color-correction placeholder).
static void BuildPortMenu(HMENU menubar) {
  HMENU port_menu = CreatePopupMenu();

  HMENU filter_menu = CreatePopupMenu();
  WORD id = PORT_FILTER_BASE;
  for (const auto& filter : kPortFilters) {
    AppendMenu(filter_menu, MF_STRING, id++, filter.title);
  }
  AppendMenu(port_menu, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>(filter_menu), "Filter");

  HMENU scale_menu = CreatePopupMenu();
  id = PORT_SCALE_BASE;
  for (const auto& scale : kPortScales) {
    AppendMenu(scale_menu, MF_STRING, id++, scale.title);
  }
  AppendMenu(port_menu, MF_POPUP, reinterpret_cast<UINT_PTR>(scale_menu), "Scale");
  AppendMenu(port_menu, MF_STRING, PORT_ASPECT_LOCK, "Lock Aspect Ratio");

  HMENU gamma_menu = CreatePopupMenu();
  WORD gid = PORT_GAMMA_BASE;
  for (const auto& g : kPortGammaOptions) {
    AppendMenu(gamma_menu, MF_STRING, gid++, g.title);
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
    PortCmdKind kind;
    int index;
    if (!PortCommandKindIndex(static_cast<WORD>(cmd), &kind, &index)) {
      continue;
    }
    int checked = 0, enabled = 1;
    PortMenu_ItemState(kind, index, &checked, &enabled);
    EnableMenuItem(menu, pos, MF_BYPOSITION | (enabled ? MF_ENABLED : MF_GRAYED));
    CheckMenuItem(menu, pos, MF_BYPOSITION | (checked ? MF_CHECKED : MF_UNCHECKED));
  }
}

// Routes a Port menu command to the cross-platform window backend.
static void HandlePortCommand(WORD cmd) {
  PortCmdKind kind;
  int index;
  if (PortCommandKindIndex(cmd, &kind, &index)) {
    PortMenu_Apply(kind, index);
  }
}

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

void WinMenuSync(SDL_Window* sdl_window, std::shared_ptr<WinMenuList> menu_list, void (*callback)(int16_t, int16_t)) {
  // Update current menu click callback function
  menuCallback = callback;

  // Store the current menu list for keyboard shortcut lookup
  current_menu_list = menu_list;

  auto wind_handle = get_window_handle(sdl_window);

  // Capture the current logical size so it can be reapplied after the menu bar is
  // attached (see the note below). Reapplying the current size rather than a fixed
  // 800x600 keeps a scale chosen from the Port menu from being reset on the next sync.
  int client_w = 800, client_h = 600;
  SDL_GetWindowSize(sdl_window, &client_w, &client_h);

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
        .fState = MFS_ENABLED,
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
  // is the full size it expects. Reapply the size captured above rather than a fixed 800x600 so a
  // scale picked from the Port menu survives a menu re-sync.
  SDL_SetWindowSize(sdl_window, client_w, client_h);
}

int WinCreatePopupMenu(SDL_Window* sdl_window, std::shared_ptr<WinMenu> menu, int window_x, int window_y) {
  auto wind_handle = get_window_handle(sdl_window);

  HMENU popupMenu = CreatePopupMenu();

  int i{0};
  for (const auto& item : menu->items) {
    i++;
    auto name = item.name.c_str();
    AppendMenu(popupMenu, (item.enabled ? MF_ENABLED : 0) | MF_STRING, i, name);
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

  return result;
}
