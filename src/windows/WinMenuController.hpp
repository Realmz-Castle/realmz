#include "SDL3/SDL_video.h"
#include <list>
#include <string>
#include <vector>

// Same as MenuManager's Menu, but without references to ResourceFile
struct WinMenu {
  struct Item {
    std::string name;
    uint8_t icon_number;
    char key_equivalent;
    char mark_character; // In MacRoman; use decode_mac_roman if needed
    uint8_t style_flags; // See TextStyleFlag
    bool enabled;
    bool checked;
  };

  int16_t menu_id;
  int16_t proc_id;
  std::string title;
  bool enabled;
  std::vector<Item> items;
};

struct WinMenuList {
  std::list<std::shared_ptr<WinMenu>> menus;
  std::list<std::shared_ptr<WinMenu>> submenus;
};

void WinMenuSync(SDL_Window* sdl_window, std::shared_ptr<WinMenuList> menu_list, void (*callback)(int16_t, int16_t));