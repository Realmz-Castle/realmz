// Menu bar and popup menus drawn with SDL inside the game window, for platforms that have no
// native menu bar (Linux). Implements the MenuController interface used by MenuManager.
//
// Menus are tracked modally, as with the classic Mac Menu Manager: while a menu is open, this code
// runs its own event loop and the game is paused. A menu opens on mouse down and an item is chosen
// by releasing over it; a quick click (release without moving) leaves the menu open so the item can
// be clicked instead.

#include "MenuBar.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cctype>
#include <format>
#include <memory>
#include <optional>
#include <resource_file/TextCodecs.hh>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Font.hpp"
#include "../MenuController.h"
#include "../PortMenu.hpp"
#include "../WindowManager.hpp"

phosg::ImageRGBA8888N DecodeCIconImage(int16_t iconID);

namespace {

using Image = phosg::ImageRGBA8888N;

// Classic Mac marks a hierarchical menu parent with command char 0x1B; its mark_character then
// holds the submenu's menu id (Macintosh Toolbox Essentials 3-96). Mark 19 is the check/diamond.
constexpr char kHierMenuCmd = 0x1B;
constexpr char kDiamondMark = 19;

constexpr int kFontSize = 12;
constexpr int kBarLeft = 8;
constexpr int kTitlePadding = 8;
constexpr int kItemHeight = 16;
constexpr int kSeparatorHeight = 8;
constexpr int kMarkGutter = 14;
constexpr int kIconGap = 4;
constexpr int kShortcutGap = 20;
constexpr int kArrowWidth = 5;
constexpr int kRightPadding = 10;
constexpr int kMinPanelWidth = 60;
constexpr int kDragThreshold = 3;

constexpr uint32_t kBlack = 0x000000FF;
constexpr uint32_t kWhite = 0xFFFFFFFF;
constexpr uint32_t kGray = 0x808080FF;

enum class Mark { None, Check, Mixed };

struct Panel;

struct Entry {
  std::string label; // UTF-8
  std::string shortcut;
  bool enabled = true;
  bool separator = false;
  uint8_t style = 0; // TextStyleFlag bits
  Mark mark = Mark::None;
  int16_t icon_id = 0;
  std::shared_ptr<Panel> submenu;
  // What choosing the entry does: a Port menu action if port_id >= 0, else a game menu item.
  int16_t menu_id = 0;
  int16_t item_id = 0;
  int port_id = -1;

  bool selectable() const {
    return this->enabled && !this->separator && !this->submenu;
  }
};

struct Panel {
  std::string title;
  bool enabled = true;
  std::vector<Entry> entries;
};

std::shared_ptr<MenuList> g_menu_list;
void (*g_callback)(int16_t, int16_t) = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Text

TTF_Font* menu_font(uint8_t style) {
  Font font = load_font(CHICAGO_FONT_ID);
  auto* tt_font = std::get_if<TTF_Font*>(&font);
  if (!tt_font) {
    return nullptr;
  }
  TTF_FontStyleFlags flags = TTF_STYLE_NORMAL;
  if (style & 0x01) {
    flags |= TTF_STYLE_BOLD;
  }
  if (style & 0x02) {
    flags |= TTF_STYLE_ITALIC;
  }
  if (style & 0x04) {
    flags |= TTF_STYLE_UNDERLINE;
  }
  TTF_SetFontSize(*tt_font, kFontSize);
  TTF_SetFontStyle(*tt_font, flags);
  return *tt_font;
}

int text_width(const std::string& text, uint8_t style = 0) {
  TTF_Font* font = menu_font(style);
  int w = 0, h = 0;
  if (!font || text.empty() || !TTF_GetStringSize(font, text.data(), text.size(), &w, &h)) {
    return 0;
  }
  return w;
}

std::shared_ptr<Image> render_text(const std::string& text, uint32_t color, uint8_t style) {
  TTF_Font* font = menu_font(style);
  if (!font) {
    return nullptr;
  }
  SDL_Color sdl_color{
      static_cast<uint8_t>(color >> 24), static_cast<uint8_t>(color >> 16), static_cast<uint8_t>(color >> 8), 0xFF};
  SDL_Surface* rendered = TTF_RenderText_Blended(font, text.data(), text.size(), sdl_color);
  if (!rendered) {
    return nullptr;
  }
  SDL_Surface* surface = SDL_ConvertSurface(rendered, SDL_PIXELFORMAT_ARGB8888);
  SDL_DestroySurface(rendered);
  if (!surface) {
    return nullptr;
  }
  auto img = std::make_shared<Image>(surface->w, surface->h);
  for (int y = 0; y < surface->h; y++) {
    auto* row = reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(surface->pixels) + y * surface->pitch);
    for (int x = 0; x < surface->w; x++) {
      img->write(x, y, phosg::rgba8888_for_argb8888(row[x]));
    }
  }
  SDL_DestroySurface(surface);
  return img;
}

// Draws text with its left edge at x, centered vertically in the row [row_y, row_y + row_h).
// Rendered text is cached, since menus redraw the same few labels repeatedly while tracking.
void draw_text(Image& dst, const std::string& text, int x, int row_y, int row_h, uint32_t color, uint8_t style = 0) {
  static std::unordered_map<std::string, std::shared_ptr<Image>> cache;
  if (text.empty()) {
    return;
  }
  std::string key = std::format("{:08X}:{:02X}:{}", color, style, text);
  auto it = cache.find(key);
  if (it == cache.end()) {
    if (cache.size() >= 1024) {
      cache.clear();
    }
    it = cache.emplace(key, render_text(text, color, style)).first;
  }
  if (const Image* img = it->second.get()) {
    int y = std::max(0, row_y + (row_h - static_cast<int>(img->get_height())) / 2);
    dst.copy_from_with_blend(*img, x, y, img->get_width(), img->get_height(), 0, 0);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Menu model

std::shared_ptr<Panel> build_game_panel(const Menu& menu, const MenuList* list, int depth) {
  auto panel = std::make_shared<Panel>();
  panel->title = ResourceDASM::decode_mac_roman(menu.title);
  panel->enabled = menu.enabled;

  int16_t item_id = 0;
  for (const auto& item : menu.items) {
    item_id++;
    Entry& entry = panel->entries.emplace_back();
    entry.menu_id = menu.menu_id;
    entry.item_id = item_id;
    if (item.name == "-" || item.name == "(-") {
      entry.separator = true;
      entry.enabled = false;
      continue;
    }
    entry.label = ResourceDASM::decode_mac_roman(item.name);
    entry.enabled = menu.enabled && item.enabled;
    entry.style = item.style_flags;
    entry.icon_id = item.icon_id;

    if (item.key_equivalent == kHierMenuCmd && item.mark_character) {
      if (list && depth < 4) {
        for (const auto& sub : list->submenus) {
          if (sub->menu_id == static_cast<uint8_t>(item.mark_character)) {
            entry.submenu = build_game_panel(*sub, list, depth + 1);
            break;
          }
        }
      }
    } else {
      if (item.checked || item.mark_character == kDiamondMark) {
        entry.mark = Mark::Check;
      } else if (item.mark_character) {
        entry.mark = Mark::Mixed;
      }
      if (item.key_equivalent) {
        entry.shortcut = std::format("Ctrl+{:c}", toupper(item.key_equivalent));
      }
    }
  }
  return panel;
}

Entry port_entry(int port_id, const char* title) {
  int checked = 0, enabled = 1;
  PortMenu_ItemState(port_id, &checked, &enabled);
  Entry entry;
  entry.label = title;
  entry.enabled = enabled;
  entry.mark = checked ? Mark::Check : Mark::None;
  entry.port_id = port_id;
  return entry;
}

template <typename OptionT, size_t Count>
Entry port_submenu(const char* title, int first_id, const OptionT (&options)[Count]) {
  Entry entry;
  entry.label = title;
  entry.submenu = std::make_shared<Panel>();
  for (size_t i = 0; i < Count; i++) {
    entry.submenu->entries.emplace_back(port_entry(first_id + i, options[i].title));
  }
  return entry;
}

// Same layout as the Port menu on macOS and Windows. Built each time menus open, so the item
// states are current.
std::shared_ptr<Panel> build_port_panel() {
  auto panel = std::make_shared<Panel>();
  panel->title = "Port";
  panel->entries.emplace_back(port_submenu("Filter", kPortFilterId, kPortFilters));
  panel->entries.emplace_back(port_submenu("Scale", kPortScaleId, kPortScales));
  panel->entries.emplace_back(port_entry(kPortAspectLockId, "Lock Aspect Ratio"));
  panel->entries.emplace_back().separator = true;
  panel->entries.emplace_back(port_submenu("Color Correction", kPortGammaId, kPortGammaOptions));
  return panel;
}

////////////////////////////////////////////////////////////////////////////////
// Menu bar

struct TitleSlot {
  std::string title;
  bool enabled;
  int x;
  int w;
};

std::vector<TitleSlot> layout_bar() {
  std::vector<TitleSlot> slots;
  int x = kBarLeft;
  auto add = [&](const std::string& title, bool enabled) {
    int w = text_width(title) + 2 * kTitlePadding;
    slots.emplace_back(TitleSlot{title, enabled, x, w});
    x += w;
  };
  if (g_menu_list) {
    for (const auto& menu : g_menu_list->menus) {
      add(ResourceDASM::decode_mac_roman(menu->title), menu->enabled);
    }
  }
  add("Port", true);
  return slots;
}

void draw_bar(Image& dst, const std::vector<TitleSlot>& slots, int highlighted) {
  int w = dst.get_width();
  dst.write_rect(0, 0, w, kMenuBarHeight - 1, kWhite);
  dst.draw_horizontal_line(0, w - 1, kMenuBarHeight - 1, 0, kBlack);
  for (size_t i = 0; i < slots.size(); i++) {
    const auto& slot = slots[i];
    bool hl = (static_cast<int>(i) == highlighted);
    if (hl) {
      dst.write_rect(slot.x, 0, slot.w, kMenuBarHeight - 1, kBlack);
    }
    uint32_t color = hl ? kWhite : (slot.enabled ? kBlack : kGray);
    draw_text(dst, slot.title, slot.x + kTitlePadding, 0, kMenuBarHeight - 1, color);
  }
}

// The bar is cached, since the screen is recomposited far more often than the menus change.
std::optional<Image> g_bar_cache;

////////////////////////////////////////////////////////////////////////////////
// Menu tracking

struct OpenPanel {
  std::shared_ptr<Panel> panel;
  int x = 0, y = 0, w = 0, h = 0;
  std::vector<int> row_y;
  std::vector<int> row_h;
  int hover = -1;

  bool contains(int px, int py) const {
    return (px >= this->x) && (px < this->x + this->w) && (py >= this->y) && (py < this->y + this->h);
  }

  int row_at(int py) const {
    for (size_t i = 0; i < this->row_y.size(); i++) {
      if (py >= this->row_y[i] && py < this->row_y[i] + this->row_h[i]) {
        return this->panel->entries[i].separator ? -1 : static_cast<int>(i);
      }
    }
    return -1;
  }
};

class Session {
public:
  explicit Session(bool popup) : popup(popup) {
    auto& screen = WindowManager::instance().screen_port.data;
    this->screen_w = screen.get_width();
    this->screen_h = screen.get_height();
  }

  // Menu bar mode: one panel per bar title, plus the Port menu
  void set_bar(std::vector<std::shared_ptr<Panel>>&& panels, std::vector<TitleSlot>&& slots) {
    this->bar_panels = std::move(panels);
    this->slots = std::move(slots);
  }

  // Popup mode: a single panel at a fixed location
  void open_popup(std::shared_ptr<Panel> panel, int x, int y) {
    this->stack.emplace_back(this->layout(panel, x, y, -1));
  }

  std::optional<Entry> track(int start_x, int start_y);

private:
  bool handle_event(SDL_Event e, SDL_Renderer* renderer, int start_x, int start_y, bool& dragged, std::optional<Entry>& result);

  bool popup;
  bool repaint = false; // Set when the window needs repainting even though no menu state changed
  int screen_w, screen_h;
  std::vector<std::shared_ptr<Panel>> bar_panels;
  std::vector<TitleSlot> slots;
  int open_title = -1;
  std::vector<OpenPanel> stack;
  std::unordered_map<int16_t, std::shared_ptr<Image>> icons;

  const Image* icon(int16_t icon_id) {
    if (icon_id <= 0) {
      return nullptr;
    }
    auto it = this->icons.find(icon_id);
    if (it == this->icons.end()) {
      std::shared_ptr<Image> img;
      try {
        img = std::make_shared<Image>(DecodeCIconImage(icon_id));
      } catch (const std::exception&) {
      }
      it = this->icons.emplace(icon_id, img).first;
    }
    return it->second.get();
  }

  // flip_x is the x position to grow leftward from if the panel doesn't fit to the right
  OpenPanel layout(std::shared_ptr<Panel> panel, int x, int y, int flip_x) {
    OpenPanel op;
    op.panel = panel;
    int left_w = 0, right_w = 0, h = 1;
    for (const auto& entry : panel->entries) {
      int row_h = kSeparatorHeight;
      if (!entry.separator) {
        const Image* img = this->icon(entry.icon_id);
        int icon_w = img ? img->get_width() + kIconGap : 0;
        row_h = std::max<int>(kItemHeight, img ? img->get_height() + 2 : 0);
        left_w = std::max(left_w, icon_w + text_width(entry.label, entry.style));
        right_w = std::max(right_w, entry.submenu ? kArrowWidth : text_width(entry.shortcut));
      }
      op.row_y.push_back(h);
      op.row_h.push_back(row_h);
      h += row_h;
    }
    op.w = std::max(kMinPanelWidth, kMarkGutter + left_w + (right_w ? kShortcutGap + right_w : 0) + kRightPadding);
    op.h = h + 1;

    if (x + op.w + 1 > this->screen_w) {
      x = (flip_x >= 0) ? flip_x - op.w : this->screen_w - op.w - 1;
    }
    if (y + op.h + 1 > this->screen_h) {
      y = this->screen_h - op.h - 1;
    }
    op.x = std::max(0, x);
    op.y = std::max(this->popup ? 0 : kMenuBarHeight - 1, y);
    for (int& row_y : op.row_y) {
      row_y += op.y;
    }
    return op;
  }

  int title_at(int x, int y) const {
    if (this->popup || y < 0 || y >= kMenuBarHeight) {
      return -1;
    }
    for (size_t i = 0; i < this->slots.size(); i++) {
      if (x >= this->slots[i].x && x < this->slots[i].x + this->slots[i].w) {
        return i;
      }
    }
    return -1;
  }

  bool contains(int x, int y) const {
    if (!this->popup && y >= 0 && y < kMenuBarHeight) {
      return true;
    }
    for (const auto& op : this->stack) {
      if (op.contains(x, y)) {
        return true;
      }
    }
    return false;
  }

  const Entry* entry_at(int x, int y) const {
    for (auto it = this->stack.rbegin(); it != this->stack.rend(); it++) {
      if (it->contains(x, y)) {
        int row = it->row_at(y);
        return (row >= 0) ? &it->panel->entries[row] : nullptr;
      }
    }
    return nullptr;
  }

  void update_hover(int x, int y) {
    int title = this->title_at(x, y);
    if (title >= 0) {
      if (title != this->open_title) {
        this->open_title = title;
        this->stack.clear();
        this->stack.emplace_back(this->layout(this->bar_panels[title], this->slots[title].x, kMenuBarHeight - 1, -1));
      } else if (!this->stack.empty()) {
        this->stack.resize(1);
        this->stack[0].hover = -1;
      }
      return;
    }

    for (int level = static_cast<int>(this->stack.size()) - 1; level >= 0; level--) {
      if (!this->stack[level].contains(x, y)) {
        continue;
      }
      int row = this->stack[level].row_at(y);
      if ((level + 1 < static_cast<int>(this->stack.size())) && (row == this->stack[level].hover)) {
        return; // Still on the parent of the open submenu
      }
      this->stack.resize(level + 1);
      this->stack[level].hover = row;
      if (row >= 0) {
        const OpenPanel& op = this->stack[level];
        const Entry& entry = op.panel->entries[row];
        if (entry.submenu && entry.enabled) {
          this->stack.emplace_back(this->layout(entry.submenu, op.x + op.w - 1, op.row_y[row] - 1, op.x + 1));
        }
      }
      return;
    }

    if (!this->stack.empty()) {
      this->stack.back().hover = -1;
    }
  }

  void draw_mark(Image& dst, int x, int cy, Mark mark, uint32_t color) {
    if (mark == Mark::Check) {
      // A small check mark, two pixels thick
      static constexpr int kCheck[][2] = {{0, 0}, {1, 1}, {2, 2}, {3, 1}, {4, 0}, {5, -1}, {6, -2}, {7, -3}};
      for (const auto& p : kCheck) {
        dst.write_rect(x + p[0], cy + p[1], 1, 2, color);
      }
    } else if (mark == Mark::Mixed) {
      dst.write_rect(x + 1, cy, 6, 2, color);
    }
  }

  void draw_arrow(Image& dst, int x, int cy, uint32_t color) {
    for (int dy = -4; dy <= 4; dy++) {
      dst.draw_horizontal_line(x, x + kArrowWidth - 1 - std::abs(dy), cy + dy, 0, color);
    }
  }

  void draw_panel(Image& dst, const OpenPanel& op) {
    // Body, border, and the classic one-pixel drop shadow
    dst.write_rect(op.x, op.y, op.w, op.h, kWhite);
    dst.draw_horizontal_line(op.x, op.x + op.w - 1, op.y, 0, kBlack);
    dst.draw_horizontal_line(op.x, op.x + op.w - 1, op.y + op.h - 1, 0, kBlack);
    dst.draw_vertical_line(op.x, op.y, op.y + op.h - 1, 0, kBlack);
    dst.draw_vertical_line(op.x + op.w - 1, op.y, op.y + op.h - 1, 0, kBlack);
    dst.draw_horizontal_line(op.x + 1, op.x + op.w, op.y + op.h, 0, kBlack);
    dst.draw_vertical_line(op.x + op.w, op.y + 1, op.y + op.h, 0, kBlack);

    for (size_t i = 0; i < op.panel->entries.size(); i++) {
      const Entry& entry = op.panel->entries[i];
      int row_y = op.row_y[i], row_h = op.row_h[i];
      if (entry.separator) {
        dst.draw_horizontal_line(op.x + 1, op.x + op.w - 2, row_y + row_h / 2, 1, kGray);
        continue;
      }
      bool hl = (static_cast<int>(i) == op.hover) && entry.enabled;
      if (hl) {
        dst.write_rect(op.x + 1, row_y, op.w - 2, row_h, kBlack);
      }
      uint32_t color = hl ? kWhite : (entry.enabled ? kBlack : kGray);
      int cy = row_y + row_h / 2;
      this->draw_mark(dst, op.x + 3, cy, entry.mark, color);

      int x = op.x + kMarkGutter;
      if (const Image* img = this->icon(entry.icon_id)) {
        dst.copy_from_with_blend(*img, x, row_y + (row_h - img->get_height()) / 2, img->get_width(), img->get_height(), 0, 0);
        x += img->get_width() + kIconGap;
      }
      draw_text(dst, entry.label, x, row_y, row_h, color, entry.style);

      int right = op.x + op.w - kRightPadding;
      if (entry.submenu) {
        this->draw_arrow(dst, right - kArrowWidth, cy, color);
      } else if (!entry.shortcut.empty()) {
        draw_text(dst, entry.shortcut, right - text_width(entry.shortcut), row_y, row_h, color);
      }
    }
  }

  // Everything that affects what is drawn: the open title, plus each open panel and its hover row
  std::vector<std::pair<const Panel*, int>> visible_state() const {
    std::vector<std::pair<const Panel*, int>> state{{nullptr, this->open_title}};
    for (const auto& op : this->stack) {
      state.emplace_back(op.panel.get(), op.hover);
    }
    return state;
  }

  void draw(Image& dst) {
    if (!this->popup) {
      draw_bar(dst, this->slots, this->open_title);
    }
    for (const auto& op : this->stack) {
      this->draw_panel(dst, op);
    }
  }
};

std::optional<Entry> Session::track(int start_x, int start_y) {
  auto& wm = WindowManager::instance();
  auto& screen = wm.screen_port.data;
  SDL_Renderer* renderer = SDL_GetRenderer(wm.get_sdl_window().get());
  Image background = screen.copy();

  // A release before the mouse moves or is pressed again is a click on the title (or on the popup's
  // control); it leaves the menu open instead of choosing whatever happens to be under the cursor.
  bool dragged = false;
  std::optional<Entry> result;
  this->update_hover(start_x, start_y);

  // Redraw only when something visible changed, and only after handling every queued event. Mouse
  // motion arrives far faster than frames can be presented, so redrawing per event makes the menus
  // lag behind the cursor.
  std::vector<std::pair<const Panel*, int>> drawn_state;
  bool needs_redraw = true;
  for (bool done = false; !done;) {
    if (needs_redraw) {
      screen.copy_from(background, 0, 0, this->screen_w, this->screen_h, 0, 0);
      this->draw(screen);
      wm.present_screen();
      drawn_state = this->visible_state();
      needs_redraw = false;
    }

    SDL_Event e;
    if (!SDL_WaitEvent(&e)) {
      break;
    }
    do {
      done = this->handle_event(e, renderer, start_x, start_y, dragged, result);
    } while (!done && SDL_PollEvent(&e));

    needs_redraw = this->repaint || (this->visible_state() != drawn_state);
    this->repaint = false;
  }

  screen.copy_from(background, 0, 0, this->screen_w, this->screen_h, 0, 0);
  wm.present_screen();
  return result;
}

// Handles one event while tracking. Returns true when tracking should end.
bool Session::handle_event(SDL_Event e, SDL_Renderer* renderer, int start_x, int start_y, bool& dragged, std::optional<Entry>& result) {
  bool done = false;
  SDL_Event original = e;
  if (renderer) {
    SDL_ConvertEventToRenderCoordinates(renderer, &e);
  }
  switch (e.type) {
    case SDL_EVENT_QUIT:
      SDL_PushEvent(&original); // Let the game's event loop handle it
      done = true;
      break;
    case SDL_EVENT_MOUSE_MOTION:
      if (std::abs(e.motion.x - start_x) >= kDragThreshold || std::abs(e.motion.y - start_y) >= kDragThreshold) {
        dragged = true;
      }
      this->update_hover(e.motion.x, e.motion.y);
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      if (e.button.button == SDL_BUTTON_LEFT) {
        if (!this->contains(e.button.x, e.button.y)) {
          done = true;
        } else {
          dragged = true;
          this->update_hover(e.button.x, e.button.y);
        }
      }
      break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      if (e.button.button == SDL_BUTTON_LEFT && dragged) {
        this->update_hover(e.button.x, e.button.y);
        const Entry* entry = this->entry_at(e.button.x, e.button.y);
        if (entry && entry->selectable()) {
          result = *entry;
          done = true;
        } else if (!this->contains(e.button.x, e.button.y)) {
          done = true;
        }
      }
      break;
    case SDL_EVENT_KEY_DOWN:
      if (e.key.key == SDLK_ESCAPE) {
        done = true;
      }
      break;
    case SDL_EVENT_WINDOW_EXPOSED:
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      this->repaint = true;
      break;
    default:
      break;
  }
  return done;
}

// Returns {menu_id, item_id} of the enabled item with the given key equivalent, or {0, 0}
std::pair<int16_t, int16_t> find_item_by_key_equivalent(char ch) {
  if (!g_menu_list) {
    return {0, 0};
  }
  ch = toupper(ch);
  for (const auto& menu_set : {g_menu_list->menus, g_menu_list->submenus}) {
    for (const auto& menu : menu_set) {
      if (!menu->enabled) {
        continue;
      }
      for (size_t z = 0; z < menu->items.size(); z++) {
        const auto& item = menu->items[z];
        if (item.enabled && item.key_equivalent != kHierMenuCmd && toupper(item.key_equivalent) == ch) {
          return {menu->menu_id, static_cast<int16_t>(z + 1)};
        }
      }
    }
  }
  return {0, 0};
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Public interface

void MenuBar_Draw(phosg::ImageRGBA8888N& screen) {
  if (!g_menu_list) {
    return;
  }
  int w = screen.get_width();
  if (!g_bar_cache || static_cast<int>(g_bar_cache->get_width()) != w) {
    g_bar_cache.emplace(w, kMenuBarHeight);
    draw_bar(*g_bar_cache, layout_bar(), -1);
  }
  screen.copy_from(*g_bar_cache, 0, 0, w, kMenuBarHeight, 0, 0);
}

bool MenuBar_HandleEvent(const SDL_Event& e) {
  if (!g_menu_list) {
    return false;
  }

  if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT &&
      e.button.y >= 0 && e.button.y < kMenuBarHeight) {
    std::vector<std::shared_ptr<Panel>> panels;
    for (const auto& menu : g_menu_list->menus) {
      panels.emplace_back(build_game_panel(*menu, g_menu_list.get(), 0));
    }
    panels.emplace_back(build_port_panel());

    Session session(false);
    session.set_bar(std::move(panels), layout_bar());
    if (auto entry = session.track(e.button.x, e.button.y)) {
      if (entry->port_id >= 0) {
        PortMenu_Apply(entry->port_id);
      } else if (g_callback) {
        g_callback(entry->menu_id, entry->item_id);
      }
    }
    return true;
  }

  // Ctrl stands in for the Mac Command key, as on Windows
  if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat && (e.key.mod & SDL_KMOD_CTRL) &&
      !(e.key.mod & SDL_KMOD_ALT) && e.key.key < 0x80 && isgraph(static_cast<int>(e.key.key))) {
    auto [menu_id, item_id] = find_item_by_key_equivalent(static_cast<char>(e.key.key));
    if (menu_id && g_callback) {
      g_callback(menu_id, item_id);
      return true;
    }
  }

  return false;
}

void MCSync(std::shared_ptr<MenuList> menuList, void (*callback)(int16_t, int16_t)) {
  g_menu_list = menuList;
  g_callback = callback;
  g_bar_cache.reset();
  WindowManager::instance().recomposite_all();
}

void MCCreatePopupMenu(
    void*, // unused
    std::shared_ptr<Menu> menu,
    std::pair<int16_t, int16_t> loc,
    void (*callback)(int16_t, int16_t)) {
  // loc is {top, left} in window coordinates; menus are tracked in render coordinates
  SDL_Window* window = WindowManager::instance().get_sdl_window().get();
  SDL_Renderer* renderer = SDL_GetRenderer(window);
  float x = loc.second, y = loc.first;
  float mouse_x = x, mouse_y = y;
  SDL_GetMouseState(&mouse_x, &mouse_y);
  if (renderer) {
    SDL_RenderCoordinatesFromWindow(renderer, loc.second, loc.first, &x, &y);
    SDL_RenderCoordinatesFromWindow(renderer, mouse_x, mouse_y, &mouse_x, &mouse_y);
  }

  Session session(true);
  session.open_popup(build_game_panel(*menu, g_menu_list.get(), 0), x, y);
  auto entry = session.track(mouse_x, mouse_y);
  callback(menu->menu_id, entry ? entry->item_id : 0);
}
