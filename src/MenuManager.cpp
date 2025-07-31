#include "MenuManager.hpp"
#include "EventManager.h"
#include "MemoryManager.hpp"
#include "MenuManager-C-Interface.h"
#include "ResourceManager.h"
#include "StringConvert.hpp"
#include "macos/MenuController.h"
#include <list>
#include <phosg/Strings.hh>
#include <resource_file/ResourceFile.hh>
#include <stdexcept>

using ResourceDASM::ResourceFile;

static phosg::PrefixedLogger mm_log("[MenuManager] ");

class MenuManager {
public:
  MenuManager() = default;
  ~MenuManager() = default;

  std::shared_ptr<Menu> get_menu(int16_t res_id) {
    if (!this->res_id_to_menu.contains(res_id)) {
      mm_log.info_f("Loading MENU:{} from resource forks", res_id);
      auto handle = GetResource(ResourceDASM::RESOURCE_TYPE_MENU, res_id);
      auto decoded_menu = ResourceFile::decode_MENU(*handle, GetHandleSize(handle));
      auto menu = std::make_shared<Menu>(Menu(decoded_menu));
      this->res_id_to_menu.emplace(res_id, menu);
      this->handle_to_menu.emplace(handle, menu);
      this->menu_id_to_handle.emplace(menu->menu_id, handle);
    }
    return this->res_id_to_menu.at(res_id);
  }

  std::shared_ptr<Menu> get_menu(Handle handle) {
    return this->handle_to_menu.at(handle);
  }

  void load_menu_list(Handle mbar_handle) {
    auto data = read_from_handle(mbar_handle);
    auto num_menus = data.get_u16b();
    auto menu_list = std::make_shared<MenuList>();
    for (int i = 0; i < num_menus; i++) {
      auto menu_res_id = data.get_s16b();
      menu_list->menus.emplace_back(this->get_menu(menu_res_id));
    }
    this->handle_to_menulist.emplace(mbar_handle, menu_list);
  }

  void set_menu_list(Handle mbar_handle) {
    this->cur_menu_list = this->handle_to_menulist.at(mbar_handle);
  }

  MenuHandle handle_by_menu_id(int16_t menu_id, bool currently_in_list) {
    if (this->menu_id_to_handle.contains(menu_id)) {
      return this->menu_id_to_handle.at(menu_id);
    } else {
      // This MENU has not been loaded yet, which means that it hasn't yet appeared
      // in a MBAR resource that we've loaded. For some reason, the game is still trying
      // to get a handle to the unloaded menu. To try to satisfy that, assume that the
      // resource id of the menu is the same as the given menu id, try loading the menu,
      // and return it.
      mm_log.warning_f("Attempted to get menu with ID {}, but it doesn't appear in current MBAR", menu_id);
      try {
        this->get_menu(menu_id);
        return this->menu_id_to_handle.at(menu_id);
      } catch (std::out_of_range&) {
        // The MENU resource just doesn't exist. It was probably deleted in this version
        // of the game.
        return NULL;
      }
    }
  }

  void insert_submenu(MenuHandle handle) {
    auto menu = this->get_menu(handle);
    this->cur_menu_list->submenus.emplace_back(menu);
  }

  void sync(void) {
    if (this->cur_menu_list != nullptr) {
      MCSync(this->cur_menu_list, &PushMenuEvent);
    }
  }

  void remove(int16_t menu_id) {
    auto menu_handle = this->menu_id_to_handle.find(menu_id)->second;
    auto menu = this->handle_to_menu.find(menu_handle)->second;

    this->handle_to_menu.erase(menu_handle);
    this->menu_id_to_handle.erase(menu_id);
    for (auto m = this->res_id_to_menu.begin(); m != this->res_id_to_menu.end(); m++) {
      if ((*m).second->menu_id == menu_id) {
        this->res_id_to_menu.erase(m);
        break;
      }
    }

    auto& menu_list = this->cur_menu_list->submenus;
    for (auto m = menu_list.begin(); m != menu_list.end(); m++) {
      if ((*m)->menu_id == menu_id) {
        menu_list.erase(m);
        return;
      }
    }

    menu_list = this->cur_menu_list->menus;
    for (auto m = menu_list.begin(); m != menu_list.end(); m++) {
      if ((*m)->menu_id == menu_id) {
        menu_list.erase(m);
        return;
      }
    }
  }

private:
  std::shared_ptr<MenuList> cur_menu_list;
  std::unordered_map<Handle, std::shared_ptr<MenuList>> handle_to_menulist;
  std::unordered_map<MenuHandle, std::shared_ptr<Menu>> handle_to_menu;
  std::unordered_map<int16_t, std::shared_ptr<Menu>> res_id_to_menu;
  std::unordered_map<int16_t, MenuHandle> menu_id_to_handle;
};

static MenuManager mm;

Handle GetNewMBar(int16_t menuBarID) {
  mm_log.info_f("Loading MBAR:{} from resource forks", menuBarID);
  auto handle = GetResource(ResourceDASM::RESOURCE_TYPE_MBAR, menuBarID);
  mm.load_menu_list(handle);
  return handle;
}

MenuHandle GetMenuHandle(int16_t menuID) {
  return mm.handle_by_menu_id(menuID, true);
}

MenuHandle GetMenu(int16_t resourceID) {
  auto menu = mm.get_menu(resourceID);
  return mm.handle_by_menu_id(menu->menu_id, false);
}

void SetMenuBar(Handle menuList) {
  mm.set_menu_list(menuList);
}

void InsertMenu(MenuHandle theMenu, int16_t beforeID) {
  if (beforeID != -1) {
    mm_log.error_f("Called InsertMenu on a non sub-menu");
    return;
  }

  mm.insert_submenu(theMenu);
}

void GetMenuItemText(MenuHandle theMenu, uint16_t item, Str255 itemString) {
  auto menu = mm.get_menu(theMenu);
  auto menu_item = menu->items[item - 1];
  pstr_for_string<256>(itemString, menu_item.name);
}

void DrawMenuBar(void) {
  mm.sync();
}

void DeleteMenu(int16_t menuID) {
  mm.remove(menuID);
}

void SetMenuItemText(MenuHandle theMenu, uint16_t item, ConstStr255Param itemString) {
  auto menu = mm.get_menu(theMenu);
  if (item > menu->items.size()) {
    mm_log.info_f("Tried to set text of menu item {} on menu {} but it only has {} items", item, menu->title, menu->items.size());
    return;
  }
  menu->items.at(item - 1).name = string_for_pstr<256>(itemString);
}

int32_t MenuSelect(Point startPt) {
  int16_t menu_id = -startPt.v;
  int16_t item_id = -startPt.h;
  mm_log.info_f("Clicked menu {}, item {}", menu_id, item_id);
  return (static_cast<int32_t>(menu_id) << 16) + item_id;
}

void DisableItem(MenuHandle theMenu, uint16_t item) {
  auto menu = mm.get_menu(theMenu);
  if (item == 0) {
    menu->enabled = false;
  } else {
    if (item > menu->items.size()) {
      mm_log.warning_f("Attempted to disable MENU:{} item {}, but it doesn't exist", menu->menu_id, item);
    } else {
      menu->items[item - 1].enabled = false;
    }
  }
}

void EnableItem(MenuHandle theMenu, uint16_t item) {
  auto menu = mm.get_menu(theMenu);
  if (item == 0) {
    menu->enabled = true;
  } else {
    if (item > menu->items.size()) {
      mm_log.warning_f("Attempted to enable MENU:{} item {}, but it doesn't exist", menu->menu_id, item);
    } else {
      menu->items[item - 1].enabled = true;
    }
  }
}

void CheckItem(MenuHandle theMenu, uint16_t item, Boolean checked) {
  auto menu = mm.get_menu(theMenu);
  if (item > menu->items.size()) {
    mm_log.warning_f("Attempted to (un)check MENU:{} item {}, but it doesn't exist", menu->menu_id, item);
  } else {
    menu->items.at(item - 1).checked = checked;
  }
}

void AppendMenu(MenuHandle menu, ConstStr255Param data) {
  auto m = mm.get_menu(menu);
  // TODO: Parse menu item format string (Macintosh Toolbox Essentials, 3-65)
  auto s = string_for_pstr<256>(data);
  auto& item = m->items.emplace_back();
  item.name = s;
}

int32_t PopUpMenuSelect(MenuHandle menu, int16_t top, int16_t left, int16_t popUpItem) {
  return 0;
}

int16_t CountMItems(MenuHandle theMenu) {
  auto m = mm.get_menu(theMenu);
  return m->items.size();
}