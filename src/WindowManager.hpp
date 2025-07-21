#pragma once

#include "WindowManager.h"

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "QuickDraw.hpp"
#include "SDLHelpers.hpp"

class WindowManager;
class Window;
class DialogItem;

class Window : public std::enable_shared_from_this<Window> {
private:
  phosg::PrefixedLogger log;
  std::string title;
  CCGrafPort port;
  int16_t window_kind;
  bool visible;
  bool is_dialog_flag;
  std::vector<std::shared_ptr<DialogItem>> dialog_items; // All items (the below 3 vectors are disjoint subsets of this)
  std::vector<std::shared_ptr<DialogItem>> static_items;
  std::vector<std::shared_ptr<DialogItem>> control_items;
  std::vector<std::shared_ptr<DialogItem>> text_items;
  std::shared_ptr<DialogItem> focused_item;
  std::shared_ptr<Window> window_below;
  std::shared_ptr<Window> window_above;

  Window(
      const std::string& title,
      const Rect& bounds,
      const CCGrafPort& parent_port,
      int16_t window_kind,
      bool visible,
      bool is_dialog,
      std::vector<std::shared_ptr<DialogItem>>&& dialog_items);

public:
  static std::shared_ptr<Window> make_shared(
      const std::string& title,
      const Rect& bounds,
      const CCGrafPort& parent_port,
      int16_t window_kind,
      bool visible,
      bool is_dialog,
      std::vector<std::shared_ptr<DialogItem>>&& dialog_items);
  ~Window() = default;

  inline std::string ref() const {
    return std::format("W-{:016X}", reinterpret_cast<intptr_t>(this));
  }

  inline const Rect& bounds() const {
    return this->port.portRect;
  }
  inline Rect& bounds() {
    return this->port.portRect;
  }
  inline size_t get_width() {
    return this->port.portRect.right - this->port.portRect.left;
  }
  inline size_t get_height() {
    return this->port.portRect.bottom - this->port.portRect.top;
  }

  void add_dialog_item(std::shared_ptr<DialogItem> item);
  std::shared_ptr<DialogItem> get_focused_item();
  CCGrafPort& get_port();
  void set_focused_item(std::shared_ptr<DialogItem> item);
  void handle_text_input(const std::string& text, std::shared_ptr<DialogItem> item);
  void delete_char(std::shared_ptr<DialogItem> item);
  void erase_and_render();
  void move(int hGlobal, int vGlobal);
  void resize(uint16_t w, uint16_t h);
  void show();
  const std::vector<std::shared_ptr<DialogItem>>& get_dialog_items() const;
  std::shared_ptr<DialogItem> dialog_item_for_position(const Point& pt, bool enabled_only);
  inline bool is_dialog() const;
  TEHandle add_text_edit(const Rect& dest_rect, const Rect& view_rect);
  void remove_text_edit(std::shared_ptr<DialogItem> item);

  friend class WindowManager;
};

class WindowManager {
private:
  std::unordered_map<DialogItemHandle, std::shared_ptr<DialogItem>> dialog_items_by_handle;
  std::unordered_map<WindowPtr, std::shared_ptr<Window>> port_to_window;
  std::shared_ptr<Window> top_window;
  std::shared_ptr<Window> bottom_window;
  sdl_window_shared sdl_window;
  phosg::ImageRGBA8888N sdl_window_data;
  bool text_editing_active;

  WindowManager();

public:
  static WindowManager& instance();
  ~WindowManager();
  void create_sdl_window();
  WindowPtr create_window(
      const std::string& title,
      const Rect& bounds,
      bool visible,
      bool go_away,
      int16_t proc_id,
      uint32_t ref_con,
      bool is_dialog,
      std::vector<std::shared_ptr<DialogItem>>&& dialog_items);
  void destroy_window(WindowPtr port);
  std::shared_ptr<Window> window_for_port(WindowPtr port);
  std::shared_ptr<DialogItem> dialog_item_for_handle(DialogItemHandle handle);
  std::shared_ptr<Window> front_window();
  void link_window_at_front(std::shared_ptr<Window> window);
  void unlink_window(std::shared_ptr<Window> window);
  void bring_to_front(std::shared_ptr<Window> window);
  std::shared_ptr<Window> window_for_point(ssize_t x, ssize_t y);

  void on_dialog_item_focus_changed();

  void recomposite(std::shared_ptr<Window> updated_window);

  // Recomposites the window stack, starting with the given window. Windows below
  // the given window are not recomposited. Updates the SDL window with the
  // rendered result.
  void recomposite_from_window(CCGrafPort& updated_port);
  void recomposite_from_window(std::shared_ptr<Window> updated_window);
  void recomposite_all();

  void on_debug_signal();

private:
  void print_window_stack() const;
  void verify_window_stack() const;
};
