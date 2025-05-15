#pragma once

#include "WindowManager.h"

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "GraphicsCanvas.hpp"
#include "SDLHelpers.hpp"

class WindowManager;
class Window;
class DialogItem;

class Window : public std::enable_shared_from_this<Window> {
private:
  std::string title;
  Rect bounds;
  int w;
  int h;
  CWindowRecord cWindowRecord;
  sdl_window_shared sdl_window;
  sdl_window_shared base_window;
  std::shared_ptr<GraphicsCanvas> canvas;
  bool is_dialog_flag;
  std::vector<std::shared_ptr<DialogItem>> dialog_items; // All items (the below 3 vectors are disjoint subsets of this)
  std::vector<std::shared_ptr<DialogItem>> static_items;
  std::vector<std::shared_ptr<DialogItem>> control_items;
  std::vector<std::shared_ptr<DialogItem>> text_items;
  std::shared_ptr<DialogItem> focused_item;
  bool text_editing_active;

public:
  Window();
  Window(
      std::string title,
      const Rect& bounds,
      CWindowRecord record,
      bool is_dialog,
      std::vector<std::shared_ptr<DialogItem>>&& dialog_items,
      sdl_window_shared base_window);
  ~Window();
  void init();
  void init_text_editing(SDL_Rect r);
  void add_dialog_item(std::shared_ptr<DialogItem> item);
  std::shared_ptr<DialogItem> get_focused_item();
  CGrafPtr get_port();
  void set_focused_item(std::shared_ptr<DialogItem> item);
  void handle_text_input(const std::string& text, std::shared_ptr<DialogItem> item);
  void delete_char(std::shared_ptr<DialogItem> item);
  void render(bool renderDialogItems = true);
  void move(int hGlobal, int vGlobal);
  void resize(uint16_t w, uint16_t h);
  void show();
  void bring_to_front();
  SDL_WindowID sdl_window_id() const;
  const std::vector<std::shared_ptr<DialogItem>>& get_dialog_items() const;
  std::shared_ptr<DialogItem> dialog_item_for_position(const Point& pt, bool enabled_only);
  inline bool is_dialog() const;
  TEHandle add_text_edit(const Rect& dest_rect, const Rect& view_rect);
  void remove_text_edit(std::shared_ptr<DialogItem> item);
};

class WindowManager {
private:
  std::unordered_map<DialogItemHandle, std::shared_ptr<DialogItem>> dialog_items_by_handle;
  std::unordered_map<WindowPtr, std::shared_ptr<Window>> record_to_window;
  std::unordered_map<SDL_WindowID, std::shared_ptr<Window>> sdl_window_id_to_window;
  std::list<std::shared_ptr<Window>> window_list;
  sdl_window_shared base_window;

public:
  WindowManager();
  ~WindowManager();
  void init();
  WindowPtr create_window(
      const std::string& title,
      const Rect& bounds,
      bool visible,
      bool go_away,
      int16_t proc_id,
      uint32_t ref_con,
      bool is_dialog,
      std::vector<std::shared_ptr<DialogItem>>&& dialog_items);
  void destroy_window(WindowPtr record);
  std::shared_ptr<Window> window_for_record(WindowPtr record);
  std::shared_ptr<Window> window_for_sdl_window_id(SDL_WindowID window_id);
  std::shared_ptr<DialogItem> dialog_item_for_handle(DialogItemHandle handle);
  std::shared_ptr<Window> front_window();
  void bring_to_front(std::shared_ptr<Window> window);
};

// Triggers a refresh of the Window object associated with the provided port record.
void render_window(CGrafPtr record);