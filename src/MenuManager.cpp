#include "MenuManager.hpp"
#include "MemoryManager.hpp"
#include "MenuManager-C-Interface.h"
#include "ResourceManager.h"
#include "StringConvert.hpp"
#include "macos/MenuController.h"
#include <list>
#include <phosg/Strings.hh>
#include <resource_file/ResourceFile.hh>
#include <stdexcept>

static phosg::PrefixedLogger mm_log("[MenuManager] ");

using ResourceDASM::ResourceFile;

class MenuManager {
public:
  MenuManager() = default;
  ~MenuManager() = default;

  std::shared_ptr<ResourceFile::DecodedMenu> get_menu(int16_t res_id) {
    if (!this->res_id_to_menu.contains(res_id)) {
      mm_log.info("Loading MENU:%d from resource forks", res_id);
      auto handle = GetResource(ResourceDASM::RESOURCE_TYPE_MENU, res_id);
      auto p = std::make_shared<ResourceFile::DecodedMenu>();
      *p = ResourceFile::decode_MENU(*handle, GetHandleSize(handle));
      this->res_id_to_menu.emplace(res_id, p);
      this->handle_to_menu.emplace(handle, p);
      this->menu_id_to_handle.emplace(p->menu_id, handle);
    }
    return this->res_id_to_menu.at(res_id);
  }

  std::shared_ptr<ResourceFile::DecodedMenu> get_menu(Handle handle) {
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
      mm_log.warning("Attempted to get menu with ID %d, but it doesn't appear in current MBAR", menu_id);
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
      MCSync(this->cur_menu_list);
    }
  }

private:
  std::shared_ptr<MenuList> cur_menu_list;
  std::unordered_map<Handle, std::shared_ptr<MenuList>> handle_to_menulist;
  std::unordered_map<MenuHandle, std::shared_ptr<ResourceFile::DecodedMenu>> handle_to_menu;
  std::unordered_map<int16_t, std::shared_ptr<ResourceFile::DecodedMenu>> res_id_to_menu;
  std::unordered_map<int16_t, MenuHandle> menu_id_to_handle;
};

static MenuManager mm;

Handle GetNewMBar(int16_t menuBarID) {
  mm_log.info("Loading MBAR:%d from resource forks", menuBarID);
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
    mm_log.error("Called InsertMenu on a non sub-menu");
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
}

void SetMenuItemText(MenuHandle theMenu, uint16_t item, ConstStr255Param itemString) {
}

int32_t MenuSelect(Point startPt) {
  return 0;
}

void DisableItem(MenuHandle theMenu, uint16_t item) {
  auto menu = mm.get_menu(theMenu);
  if (item == 0) {
    menu->enabled = false;
  } else {
    if (item > menu->items.size()) {
      mm_log.warning("Attempted to disable MENU:%d item %d, but it doesn't exist", menu->menu_id, item);
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
      mm_log.warning("Attempted to enable MENU:%d item %d, but it doesn't exist", menu->menu_id, item);
    } else {
      menu->items[item - 1].enabled = true;
    }
  }
}

void CheckItem(MenuHandle theMenu, uint16_t item, Boolean checked) {
}

void AppendMenu(MenuHandle menu, ConstStr255Param data) {
  auto menu_res = mm.get_menu(menu);
  // TODO: Parse menu item format string (Macintosh Toolbox Essentials, 3-65)
  auto s = string_for_pstr<256>(data);
  auto& item = menu_res->items.emplace_back();
  item.name = s;
}

int32_t PopUpMenuSelect(MenuHandle menu, int16_t top, int16_t left, int16_t popUpItem) {
  return 0;
}