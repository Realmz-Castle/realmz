#include "WindowManager.hpp"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_properties.h>
#include <memory>
#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <phosg/Strings.hh>
#include <resource_file/BitmapFontRenderer.hh>
#include <resource_file/ResourceFile.hh>

#include "EventManager.h"
#include "Font.hpp"
#include "MemoryManager.h"
#include "QuickDraw.hpp"
#include "ResourceManager.h"
#include "StringConvert.hpp"

using ResourceDASM::ResourceFile;

// Enable this to save an image named debug*.bmp every time the window is recomposited
static constexpr bool ENABLE_RECOMPOSITE_DEBUG = false;

inline size_t unwrap_opaque_handle(Handle h) {
  static_assert(sizeof(size_t) == sizeof(Handle));
  return reinterpret_cast<size_t>(h);
}
inline Handle wrap_opaque_handle(size_t h) {
  static_assert(sizeof(size_t) == sizeof(Handle));
  return reinterpret_cast<Handle>(h);
}

inline Rect copy_rect(const ResourceDASM::Rect& src) {
  return Rect{.top = src.y1.load(), .left = src.x1.load(), .bottom = src.y2.load(), .right = src.x2.load()};
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// SDL and rendering helpers

static phosg::PrefixedLogger wm_log("[WindowManager] ", phosg::LogLevel::L_DEBUG); // TODO: Set this back to L_INFO

static size_t generate_opaque_handle() {
  static size_t next_handle = 1;
  return next_handle++;
}

using DialogItemType = ResourceDASM::ResourceFile::DecodedDialogItem::Type;

static int16_t macos_dialog_item_type_for_resource_dasm_type(DialogItemType type) {
  switch (type) {
    case DialogItemType::BUTTON:
      return 4;
    case DialogItemType::CHECKBOX:
      return 5;
    case DialogItemType::RADIO_BUTTON:
      return 6;
    case DialogItemType::RESOURCE_CONTROL:
      return 7;
    case DialogItemType::TEXT:
      return 8;
    case DialogItemType::EDIT_TEXT:
      return 16;
    case DialogItemType::ICON:
      return 32;
    case DialogItemType::PICTURE:
      return 64;
    case DialogItemType::CUSTOM:
      return 0;
    default:
      throw std::logic_error("Unknown dialog item type");
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Controls

struct DialogItem;

enum class ControlType {
  // The values here match proc_id in the CNTL resource. There are more of
  // these, but we probably won't need them
  BUTTON = 0,
  CHECKBOX = 1,
  RADIO_BUTTON = 2,
  WINDOW_FONT_BUTTON = 8,
  WINDOW_FONT_CHECKBOX = 9,
  WINDOW_FONT_RADIO_BUTTON = 10,
  SCROLL_BAR = 16,
  POPUP_MENU = 1008,
  UNKNOWN = 0x10000,
};

// This structure is "private" (not accessible in C) like DialogItem; see the
// comment on that structure for reasoning
struct Control {
  std::weak_ptr<DialogItem> dialog_item; // May be null for dynamically-created controls!
  int32_t cntl_resource_id; // 0x00010000 = not from a resource
  size_t opaque_handle;
  ControlType type;
  Rect bounds;
  int16_t value;
  int16_t min;
  int16_t max;
  bool visible = true;
  std::string title;

protected:
  static std::shared_ptr<Control> make_shared(
      int32_t cntl_res_id,
      const Rect& bounds,
      int16_t value,
      int16_t min,
      int16_t max,
      int16_t proc_id,
      bool visible,
      const std::string& title) {
    auto ret = std::make_shared<Control>();
    ret->cntl_resource_id = cntl_res_id;
    ret->opaque_handle = generate_opaque_handle();
    switch (proc_id) {
      case 0:
        ret->type = ControlType::BUTTON;
        break;
      case 1:
        ret->type = ControlType::CHECKBOX;
        break;
      case 2:
        ret->type = ControlType::RADIO_BUTTON;
        break;
      case 8:
        ret->type = ControlType::WINDOW_FONT_BUTTON;
        break;
      case 9:
        ret->type = ControlType::WINDOW_FONT_CHECKBOX;
        break;
      case 10:
        ret->type = ControlType::WINDOW_FONT_RADIO_BUTTON;
        break;
      case 16:
        ret->type = ControlType::SCROLL_BAR;
        break;
      case 1008:
        ret->type = ControlType::POPUP_MENU;
        break;
      default:
        throw std::runtime_error(std::format("Unknown control type {}", proc_id));
    }
    ret->bounds = bounds;
    ret->value = value;
    ret->min = min;
    ret->max = max;
    ret->visible = visible;
    ret->title = title;
    return ret;
  }

public:
  // Create a new control manually. This implements the NewControl syscall.
  static std::shared_ptr<Control> from_params(
      const Rect& bounds,
      int16_t value,
      int16_t min,
      int16_t max,
      int16_t proc_id,
      bool visible,
      const std::string& title) {
    return Control::make_shared(0x00010000, bounds, value, min, max, proc_id, visible, title);
  }
  // Create a new control from a resource. This implements the GetNewControl syscall.
  static std::shared_ptr<Control> from_CNTL(int16_t cntl_resource_id) {
    auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_CNTL, cntl_resource_id);
    auto def = ResourceDASM::ResourceFile::decode_CNTL(*data_handle, GetHandleSize(data_handle));
    Rect bounds = copy_rect(def.bounds);
    return Control::make_shared(cntl_resource_id, bounds, def.value, def.min, def.max, def.proc_id, def.visible, def.title);
  }
  // Create a new control from a dialog item. This implements controls
  // generated from DITL entries. Annoyingly, this can't be implemented here
  // because it depends on the internals of DialogItem, which is still an
  // incomplete type at this point.
  static std::shared_ptr<Control> from_dialog_item(const DialogItem& item);

  ~Control() = default;

  std::string str() const {
    static const std::unordered_map<ControlType, const char*> type_strs{
        {ControlType::BUTTON, "BUTTON"},
        {ControlType::CHECKBOX, "CHECKBOX"},
        {ControlType::RADIO_BUTTON, "RADIO_BUTTON"},
        {ControlType::WINDOW_FONT_BUTTON, "WINDOW_FONT_BUTTON"},
        {ControlType::WINDOW_FONT_CHECKBOX, "WINDOW_FONT_CHECKBOX"},
        {ControlType::WINDOW_FONT_RADIO_BUTTON, "WINDOW_FONT_RADIO_BUTTON"},
        {ControlType::SCROLL_BAR, "SCROLL_BAR"},
        {ControlType::POPUP_MENU, "POPUP_MENU"},
    };
    return std::format(
        "Control(cntl_resource_id={}, opaque_handle={}, type={}, value={}, min={}, max={}, visible={})",
        this->cntl_resource_id,
        this->opaque_handle,
        type_strs.at(this->type),
        this->value,
        this->min,
        this->max,
        this->visible ? "true" : "false");
  }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Dialog items

// This structure is "private" (not accessible in C) because it isn't directly
// used there: Realmz only interacts with dialog items through syscalls and
// handles, so we can use C++ types here without breaking anything.
struct DialogItem {
public:
  // Identity
  size_t opaque_handle;
  std::weak_ptr<Window> owner_window;

  // Source information
  int32_t ditl_resource_id; // 0x00010000 = not from a DITL
  size_t item_id;

  // Options
  DialogItemType type;
  int16_t resource_id; // From item definition (generally used for controls)
  Rect rect;
  bool enabled;
  std::shared_ptr<Control> control; // May be null

  static std::unordered_map<size_t, std::weak_ptr<DialogItem>> all_items;

  static std::shared_ptr<DialogItem> get_item_by_handle(size_t handle) {
    auto item = DialogItem::all_items.at(handle).lock();
    if (!item) {
      throw std::logic_error(std::format(
          "Attempted to get missing or destroyed dialog item (handle was {})", handle));
    }
    return item;
  }

private:
  bool dirty;
  std::string text;

public:
  // Constructor from a definition
  DialogItem(
      int32_t ditl_res_id,
      size_t item_id,
      const ResourceDASM::ResourceFile::DecodedDialogItem& def)
      : opaque_handle{generate_opaque_handle()},
        ditl_resource_id{ditl_res_id},
        item_id{item_id},
        type{def.type},
        resource_id{def.resource_id},
        enabled{def.enabled},
        dirty{true},
        text{def.text} {
    this->rect.left = def.bounds.x1;
    this->rect.right = def.bounds.x2;
    this->rect.top = def.bounds.y1;
    this->rect.bottom = def.bounds.y2;
    this->control = Control::from_dialog_item(*this); // May return null
  }
  // Constructor from a control
  DialogItem(std::shared_ptr<Control> control)
      : opaque_handle{control->opaque_handle},
        ditl_resource_id{0x00010000},
        item_id{0},
        type{DialogItemType::UNKNOWN},
        resource_id{0},
        rect{control->bounds},
        enabled{true},
        control{control},
        dirty{true},
        text{control->title} {
    switch (control->type) {
      case ControlType::BUTTON:
        this->type = DialogItemType::BUTTON;
        break;
      case ControlType::CHECKBOX:
        this->type = DialogItemType::CHECKBOX;
        break;
      case ControlType::RADIO_BUTTON:
        this->type = DialogItemType::RADIO_BUTTON;
        break;

      // TODO: Figure out how to implement this. We'll need it for the trade/shop screen.
      // case ControlType::SCROLL_BAR:

      // We don't support these (yet?)
      // case ControlType::WINDOW_FONT_BUTTON:
      // case ControlType::WINDOW_FONT_CHECKBOX:
      // case ControlType::WINDOW_FONT_RADIO_BUTTON:
      // case ControlType::POPUP_MENU:
      default:
        throw std::runtime_error("unsupported control type");
    }
  }

  DialogItem(DialogItemType type, const Rect& disp_rect, const Rect& view_rect)
      : opaque_handle{generate_opaque_handle()},
        type{type},
        dirty{true},
        text{""},
        enabled{true},
        rect{disp_rect} {
    if (type != DialogItemType::TEXT) {
      throw std::runtime_error("Cannot dynamically create bounded dialog item unless it is type TEXT");
    }
    if (disp_rect.top != view_rect.top ||
        disp_rect.left != view_rect.left ||
        disp_rect.bottom != view_rect.bottom ||
        disp_rect.right != view_rect.right) {
      throw std::runtime_error("Layout and clip rects of monostyled edit record do not match");
    }
  }

  // Create a list of dialog items from a DITL resource
  static std::vector<std::shared_ptr<DialogItem>> from_DITL(int16_t ditl_resource_id) {
    auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_DITL, ditl_resource_id);
    auto defs = ResourceDASM::ResourceFile::decode_DITL(*data_handle, GetHandleSize(data_handle));

    std::vector<std::shared_ptr<DialogItem>> ret;
    for (const auto& decoded_dialog_item : defs) {
      size_t item_id = ret.size() + 1;
      auto di = ret.emplace_back(new DialogItem(ditl_resource_id, item_id, decoded_dialog_item));
      all_items[di->opaque_handle] = di;
    }
    return ret;
  }

  // Create a single dialog item from a CNTL resource. This is necessary for
  // the NewControl and GetNewControl syscalls. The returned dialog item has
  // an item_id of zero, which will be overwritten by add_dialog_item.
  static std::shared_ptr<DialogItem> from_control(std::shared_ptr<Control> control) {
    auto ret = std::make_shared<DialogItem>(control);
    all_items[ret->opaque_handle] = ret;
    return ret;
  }

  static std::shared_ptr<DialogItem> from_text_edit(const Rect& dest_rect, const Rect& view_rect) {
    auto ret = std::make_shared<DialogItem>(DialogItemType::TEXT, dest_rect, view_rect);
    all_items[ret->opaque_handle] = ret;
    return ret;
  }

  ~DialogItem() {
    all_items.erase(opaque_handle);
  }

  std::string str() const {
    static const std::unordered_map<DialogItemType, const char*> type_strs{
        {DialogItemType::BUTTON, "BUTTON"},
        {DialogItemType::CHECKBOX, "CHECKBOX"},
        {DialogItemType::RADIO_BUTTON, "RADIO_BUTTON"},
        {DialogItemType::RESOURCE_CONTROL, "RESOURCE_CONTROL"},
        {DialogItemType::HELP_BALLOON, "HELP_BALLOON"},
        {DialogItemType::TEXT, "TEXT"},
        {DialogItemType::EDIT_TEXT, "EDIT_TEXT"},
        {DialogItemType::ICON, "ICON"},
        {DialogItemType::PICTURE, "PICTURE"},
        {DialogItemType::CUSTOM, "CUSTOM"},
        {DialogItemType::UNKNOWN, "UNKNOWN"},
    };
    auto text_str = phosg::format_data_string(this->text);
    auto control_str = this->control ? this->control->str() : "NULL";
    return std::format(
        "DialogItem(ditl_resource_id={}, item_id={}, type={}, resource_id={}, rect=Rect(left={}, top={}, right={}, bottom={}), enabled={}, control={}, handle={}, dirty={}, text={})",
        this->ditl_resource_id,
        this->item_id,
        type_strs.at(this->type),
        this->resource_id,
        this->rect.left,
        this->rect.top,
        this->rect.right,
        this->rect.bottom,
        this->enabled ? "true" : "false",
        control_str.c_str(),
        this->opaque_handle,
        this->dirty ? "true" : "false",
        text_str.c_str());
  }

  void render_in_port(CCGrafPort& port) const {
    if (port.bkPixPat) {
      port.draw_background_ppat(this->rect);
    } else {
      port.clear_rect(&this->rect);
    }
    switch (type) {
      case ResourceFile::DecodedDialogItem::Type::PICTURE: {
        auto pict_handle = GetPicture(resource_id);
        if (!pict_handle) {
          wm_log.warning_f("Attempted to draw PICT:{}, but it could not be loaded", resource_id);
        } else {
          port.draw_decoded_pict_from_handle(pict_handle, this->rect);
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::ICON:
        // TODO
        wm_log.warning_f("Attempted to draw ICON dialog item, but it's not implemented");
        break;
      case ResourceFile::DecodedDialogItem::Type::TEXT: {
        if (text.length() > 0 && !port.draw_text(text, this->rect)) {
          wm_log.error_f("Error when rendering text item {}: {}", resource_id, SDL_GetError());
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::BUTTON: {
        if (!port.draw_text(text, this->rect)) {
          wm_log.error_f("Error when rendering button text item {}: {}", resource_id, SDL_GetError());
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::EDIT_TEXT: {
        if (!port.draw_text(text, this->rect)) {
          wm_log.error_f("Error when rendering editable text item {}: {}", resource_id, SDL_GetError());
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::CHECKBOX:
      case ResourceFile::DecodedDialogItem::Type::RADIO_BUTTON: {
        // TODO: For now, we just draw radio buttons the same as checkboxes. (Does Realmz even use radio buttons?)
        // Draw checkbox
        const auto& r = this->rect;
        Point top_left = {.h = r.left, .v = r.top};
        Point top_right = {.h = static_cast<int16_t>(r.left + 10), .v = r.top};
        Point bottom_left = {.h = r.left, .v = static_cast<int16_t>(r.top + 10)};
        Point bottom_right = {.h = static_cast<int16_t>(r.left + 10), .v = static_cast<int16_t>(r.top + 10)};
        port.draw_line(top_left, bottom_left); // Left side
        port.draw_line(top_right, bottom_right); // Right side
        port.draw_line(top_left, top_right); // Top side
        port.draw_line(bottom_left, bottom_right); // Bottom side
        if (control && control->value) {
          // Draw an X also if the checkbox is checked
          port.draw_line(top_left, bottom_right);
          port.draw_line(top_right, bottom_left);
        }
        int16_t h = get_height();
        int16_t w = get_width();
        if (!port.draw_text(text, Rect{r.top, static_cast<int16_t>(r.left + 12), r.bottom, r.right})) {
          wm_log.error_f("Error when rendering button text item {}: {}", resource_id, SDL_GetError());
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::RESOURCE_CONTROL:
      case ResourceFile::DecodedDialogItem::Type::HELP_BALLOON:
      case ResourceFile::DecodedDialogItem::Type::CUSTOM:
      case ResourceFile::DecodedDialogItem::Type::UNKNOWN:
        // TODO: Should we draw anything for these types?
        break;
      default:
        break;
    }
  }

  inline int16_t get_width() const {
    return rect.right - rect.left;
  }

  inline int16_t get_height() const {
    return rect.bottom - rect.top;
  }

  inline const std::string& get_text() const {
    return text;
  }

  inline void set_dirty_flag() {
    dirty = true;
  }

  void set_text(const std::string& new_text) {
    text = new_text;
    if (control) {
      control->title = text;
    }
    dirty = true;
  }
  void set_text(std::string&& new_text) {
    text = std::move(new_text);
    if (control) {
      control->title = text;
    }
    dirty = true;
  }

  void append_text(const std::string& new_text) {
    text += new_text;
    dirty = true;
  }

  void delete_char() {
    if (text.size()) {
      text.pop_back();
      dirty = true;
    }
  }

  bool set_control_visible(bool visible) {
    if (this->control && (this->control->visible != visible)) {
      this->control->visible = visible;
      this->dirty = true;
    }
    return this->dirty;
  }

  bool move_control(short horizontal, short vertical) {
    if (this->control) {
      Rect& bounds = this->control->bounds;
      int32_t width = bounds.right - bounds.left;
      int32_t height = bounds.bottom - bounds.top;
      bounds.left = horizontal;
      bounds.top = vertical;
      bounds.right = bounds.left + width;
      bounds.bottom = bounds.top + height;
      this->rect = bounds;
      this->dirty = true;
    }
    return this->dirty;
  }

  bool resize_control(short w, short h) {
    if (this->control) {
      Rect& bounds = this->control->bounds;
      bounds.right = bounds.left + w;
      bounds.bottom = bounds.top + h;
      this->rect = bounds;
      this->dirty = true;
    }
    return this->dirty;
  }

  bool set_control_value(short value) {
    if (this->control && (this->control->value != value)) {
      this->control->value = value;
      this->dirty = true;
    }
    return this->dirty;
  }

  bool set_control_minimum(short min) {
    if (this->control && (this->control->min != min)) {
      this->control->min = min;
      this->control->max = std::max<int16_t>(this->control->max, this->control->min);
      this->control->value = std::max<int16_t>(this->control->min, this->control->value);
      this->dirty = true;
    }
    return this->dirty;
  }

  bool set_control_maximum(short max) {
    if (this->control && (this->control->max != max)) {
      this->control->max = max;
      this->control->min = std::min<int16_t>(this->control->max, this->control->min);
      this->control->value = std::min<int16_t>(this->control->max, this->control->value);
      this->dirty = true;
    }
    return this->dirty;
  }
};

std::unordered_map<size_t, std::weak_ptr<DialogItem>> DialogItem::all_items;

std::shared_ptr<Control> Control::from_dialog_item(const DialogItem& item) {
  ControlType type;
  switch (item.type) {
    case DialogItemType::BUTTON:
      type = ControlType::BUTTON;
      break;
    case DialogItemType::CHECKBOX:
      type = ControlType::CHECKBOX;
      break;
    case DialogItemType::RADIO_BUTTON:
      type = ControlType::RADIO_BUTTON;
      break;
    case DialogItemType::RESOURCE_CONTROL:
      return Control::from_CNTL(item.resource_id);
    default:
      return nullptr;
  }

  auto ret = std::make_shared<Control>();
  ret->cntl_resource_id = 0x00010000;
  ret->opaque_handle = item.opaque_handle;
  ret->type = type;
  ret->bounds = item.rect;
  ret->value = 0;
  ret->min = 0;
  ret->max = 1;
  ret->visible = true;
  ret->title = item.get_text();
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Windows

Window::Window(
    const std::string& title,
    const Rect& bounds,
    const CCGrafPort& parent_port,
    int16_t window_kind,
    bool visible,
    bool is_dialog,
    std::vector<std::shared_ptr<DialogItem>>&& dialog_items)
    : title{title},
      port{bounds, &parent_port, true},
      window_kind{window_kind},
      visible(visible),
      is_dialog_flag{is_dialog},
      dialog_items{std::move(dialog_items)},
      focused_item{nullptr} {
  // All windows created by Realmz should be borderless, except the first one
  bool is_borderless = ((this->window_kind == dBoxProc) ||
      (this->window_kind == plainDBox) ||
      (this->window_kind == altDBoxProc));
  if (!is_borderless) {
    phosg::fwrite_fmt(stderr, "Warning: Creating non-borderless window\n");
  }

  for (auto di : this->dialog_items) {
    // Set the focused text field to be the first EDIT_TEXT item encountered
    if (!focused_item && di->type == DialogItemType::EDIT_TEXT) {
      focused_item = di;
    }

    if (di->type == DialogItemType::TEXT || di->type == DialogItemType::EDIT_TEXT) {
      text_items.emplace_back(di);
    } else if (di->control) {
      control_items.emplace_back(di);
    } else {
      static_items.emplace_back(di);
    }
  }
}

std::shared_ptr<Window> Window::make_shared(
    const std::string& title,
    const Rect& bounds,
    const CCGrafPort& parent_port,
    int16_t window_kind,
    bool visible,
    bool is_dialog,
    std::vector<std::shared_ptr<DialogItem>>&& dialog_items) {
  std::shared_ptr<Window> ret(new Window(title, bounds, parent_port, window_kind, visible, is_dialog, std::move(dialog_items)));
  // This can't happen in the actual constructor because weak_from_this()
  // doesn't work until the Window is assigned to a shared_ptr
  for (auto& di : ret->dialog_items) {
    di->owner_window = ret;
  }
  return ret;
}

void Window::add_dialog_item(std::shared_ptr<DialogItem> item) {
  item->item_id = this->dialog_items.size();
  this->dialog_items.emplace_back(item);
  item->owner_window = this->weak_from_this();

  // TODO: Should we draw just this item, or re-render the entire window?
  wm_log.debug_f("Window::add_dialog_item({})", item->str());
  item->render_in_port(this->port);
  WindowManager::instance().recomposite_from_window(this->shared_from_this());
}

std::shared_ptr<DialogItem> Window::get_focused_item() {
  return focused_item;
}

CCGrafPort& Window::get_port() {
  return this->port;
}

void Window::set_focused_item(std::shared_ptr<DialogItem> item) {
  focused_item = item;
}

void Window::handle_text_input(const std::string& text, std::shared_ptr<DialogItem> item) {
  wm_log.debug_f("Window::handle_text_input(\"{}\", {})", text, item->str());
  item->append_text(text);
  WindowManager::instance().recomposite_from_window(this->port);
}

void Window::delete_char(std::shared_ptr<DialogItem> item) {
  wm_log.debug_f("Window::delete_char({})", item->str());
  item->delete_char();
  WindowManager::instance().recomposite_from_window(this->port);
}

void Window::erase_and_render() {
  wm_log.debug_f("Window::erase_and_render()");

  // Clear the backbuffer before drawing frame
  if (this->port.bkPixPat) {
    this->port.draw_background_ppat();
  } else {
    this->port.clear_rect(nullptr);
  }

  // The DrawDialog procedure draws the entire contents of the specified dialog box. The
  // DrawDialog procedure draws all dialog items, calls the Control Manager procedure
  // DrawControls to draw all controls, and calls the TextEdit procedure TEUpdate to
  // update all static and editable text items and to draw their display rectangles. The
  // DrawDialog procedure also calls the application-defined items’ draw procedures if
  // the items’ rectangles are within the update region.
  for (auto item : this->static_items) {
    item->render_in_port(this->port);
  }
  for (auto item : this->control_items) {
    item->render_in_port(this->port);
  }
  for (auto item : this->text_items) {
    item->render_in_port(this->port);
  }

  WindowManager::instance().recomposite_from_window(this->port);
}

void Window::move(int x, int y) {
  wm_log.debug_f("Window::move({}, {})", x, y);
  auto& bounds = this->bounds();
  ssize_t x_delta = x - bounds.left;
  ssize_t y_delta = y - bounds.top;
  if (x_delta || y_delta) {
    bounds.left += x_delta;
    bounds.right += x_delta;
    bounds.top += y_delta;
    bounds.bottom += y_delta;
    WindowManager::instance().recomposite_all();
  }
}

void Window::resize(uint16_t w, uint16_t h) {
  wm_log.debug_f("Window::resize({}, {})", w, h);

  bool shrank_either_dimension = (this->get_width() > w) || (this->get_height() > h);
  this->port.resize(w, h);

  // Recomposite everything if this window shrank in either dimension (since
  // windows behind it may be revealed); else, recomposite only this window
  if (shrank_either_dimension) {
    WindowManager::instance().recomposite_all();
  } else {
    WindowManager::instance().recomposite_from_window(this->shared_from_this());
  }
}

void Window::show() {
  wm_log.debug_f("Window::show()");
  if (!this->visible) {
    this->visible = true;
    for (auto di : dialog_items) {
      di->set_dirty_flag();
    }
    this->erase_and_render();
    WindowManager::instance().recomposite_from_window(this->port);
  }
}

const std::vector<std::shared_ptr<DialogItem>>& Window::get_dialog_items() const {
  return this->dialog_items;
}

std::shared_ptr<DialogItem> Window::dialog_item_for_position(const Point& pt, bool enabled_only) {
  for (const auto& item : this->dialog_items) {
    if ((!enabled_only || item->enabled) && PtInRect(pt, &item->rect)) {
      return item;
    }
  }
  return nullptr;
}

inline bool Window::is_dialog() const {
  return this->is_dialog_flag;
}

TEHandle Window::add_text_edit(const Rect& dest_rect, const Rect& view_rect) {
  auto di = DialogItem::from_text_edit(dest_rect, view_rect);
  add_dialog_item(di);
  text_items.emplace_back(di);
  return reinterpret_cast<TEHandle>(di->opaque_handle);
}

void Window::remove_text_edit(std::shared_ptr<DialogItem> item) {
  auto it = std::find(dialog_items.begin(), dialog_items.end(), item);
  if (it != dialog_items.end()) {
    dialog_items.erase(it);
  }
  it = std::find(text_items.begin(), text_items.end(), item);
  if (it != text_items.end()) {
    text_items.erase(it);
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Window manager

WindowManager::WindowManager() = default;
WindowManager::~WindowManager() = default;

void WindowManager::create_sdl_window() {
  wm_log.debug_f("WindowManager::create_sdl_window()");

  static constexpr size_t w = 800;
  static constexpr size_t h = 600;
  this->sdl_window = sdl_make_shared(SDL_CreateWindow("Realmz", w, h, 0));
  if (!this->sdl_window) {
    throw std::runtime_error(std::format("Could not create SDL window: {}", SDL_GetError()));
  }
  if (!SDL_CreateRenderer(this->sdl_window.get(), nullptr)) {
    throw std::runtime_error(std::format("Could not create window renderer: {}", SDL_GetError()));
  }
  this->sdl_window_data.resize(w, h);
  this->recomposite_all();
}

WindowPtr WindowManager::create_window(
    const std::string& title,
    const Rect& bounds,
    bool visible,
    bool go_away,
    int16_t proc_id,
    uint32_t ref_con,
    bool is_dialog,
    std::vector<std::shared_ptr<DialogItem>>&& dialog_items) {
  wm_log.debug_f("WindowManager::create_window(\"{}\", {{x0={}, y0={}, x1={}, y1={}}}, ...)", title, bounds.left, bounds.top, bounds.right, bounds.bottom);

  auto window = Window::make_shared(title, bounds, current_port(), proc_id, visible, is_dialog, std::move(dialog_items));
  port_to_window.emplace(&window->get_port(), window);

  // Maintain a shared lookup across all windows of their dialog items, by handle,
  // to support functions that modify the DITLs directly, like SetDialogItemText
  for (auto di : dialog_items) {
    DialogItem::all_items[di->opaque_handle] = di;
  }

  this->link_window_at_front(window);
  this->on_dialog_item_focus_changed();

  // Render the window's contents even if it isn't visible, and recomposite
  window->erase_and_render();
  recomposite_from_window(window);

  return &window->get_port();
}

void WindowManager::destroy_window(WindowPtr port) {
  wm_log.debug_f("WindowManager::destroy_window({})", reinterpret_cast<void*>(port));

  auto window_it = port_to_window.find(port);
  if (window_it == port_to_window.end()) {
    throw std::logic_error("Attempted to delete nonexistent window");
  }

  // When the window is dismissed via a mousedown, the enqueued mouseup event is either
  // lost when the window is destroyed, or the button is not released in time for it to be
  // handled by the window. It's not clear why the mouseup event isn't handled by the surviving
  // window. In any case, we can simply reset the mouse state to prevent the StillDown function
  // from thinking that the mouse button is still pressed.
  // TODO: figure out a better way of handling this.
  reset_mouse_state();

  bool should_update_focus = (window_it->second == this->top_window);
  this->unlink_window(window_it->second);
  port_to_window.erase(window_it);
  if (should_update_focus) {
    this->on_dialog_item_focus_changed();
  }

  // If the current port is this window's port, set the current port back to
  // the default port
  if (qd.thePort == port) {
    if (!default_port) {
      default_port = std::make_unique<CCGrafPort>();
    }
    SetPort(default_port.get());
  }

  // Recomposite everything, since this window many have been obstructing other windows
  this->recomposite_all();
}

std::shared_ptr<Window> WindowManager::window_for_port(WindowPtr port) {
  return this->port_to_window.at(port);
}

std::shared_ptr<DialogItem> WindowManager::dialog_item_for_handle(DialogItemHandle handle) {
  return dialog_items_by_handle.at(handle);
}

std::shared_ptr<Window> WindowManager::front_window() {
  return this->top_window;
}

void WindowManager::link_window_at_front(std::shared_ptr<Window> window) {
  window->window_below = this->top_window;
  if (this->top_window) {
    this->top_window->window_above = window;
  }
  this->top_window = window;
  if (!this->bottom_window) {
    this->bottom_window = window;
  }
}

void WindowManager::unlink_window(std::shared_ptr<Window> window) {
  if (this->top_window == window) {
    this->top_window = window->window_below;
  }
  if (this->bottom_window == window) {
    this->bottom_window = window->window_above;
  }
  if (window->window_below) {
    window->window_below->window_above = window->window_above;
    window->window_below.reset();
  }
  if (window->window_above) {
    window->window_above->window_below = window->window_below;
    window->window_above.reset();
  }
}

void WindowManager::bring_to_front(std::shared_ptr<Window> window) {
  wm_log.debug_f("WindowManager::bring_to_front({})", reinterpret_cast<void*>(window.get()));

  if (window != this->top_window) {
    this->unlink_window(window);
    this->link_window_at_front(window);
    this->on_dialog_item_focus_changed();
    if (window->visible) {
      this->recomposite_from_window(window);
    }
  }
}

std::shared_ptr<Window> WindowManager::window_for_point(ssize_t x, ssize_t y) {
  Point pt{.h = static_cast<int16_t>(x), .v = static_cast<int16_t>(y)};
  for (auto window = this->top_window; window; window = window->window_below) {
    if (PtInRect(pt, &window->port.portRect)) {
      return window;
    }
  }
  return nullptr;
}

void WindowManager::on_dialog_item_focus_changed() {
  // Macintosh Toolbox Essentials 6-32

  if (this->text_editing_active) {
    wm_log.info_f("Ending SDL text input");
    SDL_StopTextInput(this->sdl_window.get());
    this->text_editing_active = false;
  }

  if (this->top_window &&
      this->top_window->focused_item &&
      (this->top_window->focused_item->type == DialogItemType::EDIT_TEXT)) {

    wm_log.info_f("Starting SDL text input");

    const auto& window_rect = this->top_window->port.portRect;
    const auto& item_rect = this->top_window->focused_item->rect;

    SDL_Rect rect;
    rect.x = window_rect.left + item_rect.left;
    rect.y = window_rect.top + item_rect.top;
    rect.w = item_rect.right - item_rect.left;
    rect.h = item_rect.bottom - item_rect.top;
    if (!SDL_SetTextInputArea(this->sdl_window.get(), &rect, 0)) {
      wm_log.error_f("Could not create text area: {}", SDL_GetError());
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);
    SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN, false);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);

    if (!SDL_StartTextInputWithProperties(this->sdl_window.get(), props)) {
      wm_log.error_f("Could not start text input: {}", SDL_GetError());
    }

    SDL_DestroyProperties(props);

    this->text_editing_active = true;
  }
}

void WindowManager::recomposite(std::shared_ptr<Window> updated_window) {
  if (updated_window && !updated_window->visible) {
    return;
  }

  std::shared_ptr<Window> window;
  if (!updated_window) {
    this->sdl_window_data.clear(0x000000FF);
    window = this->bottom_window;
  } else {
    window = updated_window;
  }

  for (; window; window = window->window_above) {
    // Draw window border
    this->sdl_window_data.draw_horizontal_line(window->port.portRect.left - 1, window->port.portRect.right, window->port.portRect.top - 1, 0, 0x000000FF);
    this->sdl_window_data.draw_horizontal_line(window->port.portRect.left - 1, window->port.portRect.right, window->port.portRect.bottom, 0, 0x000000FF);
    this->sdl_window_data.draw_vertical_line(window->port.portRect.left - 1, window->port.portRect.top, window->port.portRect.bottom, 0, 0x000000FF);
    this->sdl_window_data.draw_vertical_line(window->port.portRect.right, window->port.portRect.top, window->port.portRect.bottom, 0, 0x000000FF);

    this->sdl_window_data.copy_from_with_blend(
        window->port.data,
        window->port.portRect.left,
        window->port.portRect.top,
        window->get_width(),
        window->get_height(),
        0,
        0);
  }

  if (this->sdl_window) {
    auto renderer = SDL_GetRenderer(this->sdl_window.get());
    if (!renderer) {
      wm_log.error_f("Could not get window renderer: {}", SDL_GetError());
    } else {

      auto w = this->sdl_window_data.get_width();
      auto h = this->sdl_window_data.get_height();
      if (ENABLE_RECOMPOSITE_DEBUG) {
        static size_t debug_number = 1;
        wm_log.info_f("Writing debug{}.bmp", debug_number);
        phosg::save_file(std::format("debug{}.bmp", debug_number++), this->sdl_window_data.serialize(phosg::ImageFormat::WINDOWS_BITMAP));
      }
      auto surface = sdl_make_unique(SDL_CreateSurfaceFrom(
          w, h, SDL_PIXELFORMAT_RGBA8888, this->sdl_window_data.get_data(), 4 * this->sdl_window_data.get_width()));
      if (!surface) {
        wm_log.error_f("Could not create surface: {}", SDL_GetError());
      } else {
        auto texture = sdl_make_unique(SDL_CreateTextureFromSurface(renderer, surface.get()));
        if (!texture) {
          wm_log.error_f("Could not create texture: {}", SDL_GetError());
        } else {
          SDL_RenderTexture(renderer, texture.get(), nullptr, nullptr);
        }
      }

      SDL_RenderPresent(renderer);
      SDL_SyncWindow(this->sdl_window.get());
    }
  }
}

WindowManager& WindowManager::instance() {
  static std::unique_ptr<WindowManager> wm;
  if (!wm) {
    wm.reset(new WindowManager());
  }
  return *wm;
}

void WindowManager::recomposite_from_window(CCGrafPort& updated_port) {
  if (updated_port.is_window) {
    this->recomposite(this->window_for_port(&updated_port));
  }
}
void WindowManager::recomposite_from_window(std::shared_ptr<Window> updated_window) {
  this->recomposite(updated_window);
}
void WindowManager::recomposite_all() {
  this->recomposite(nullptr);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Classic Mac OS API

static void SDL_snprintfcat(SDL_OUT_Z_CAP(maxlen) char* text, size_t maxlen, SDL_PRINTF_FORMAT_STRING const char* fmt, ...) {
  size_t length = SDL_strlen(text);
  va_list ap;

  va_start(ap, fmt);
  text += length;
  maxlen -= length;
  (void)SDL_vsnprintf(text, maxlen, fmt, ap);
  va_end(ap);
}

static void PrintDebugInfo(void) {
  int i, n;
  char text[1024];

  SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
  n = SDL_GetNumVideoDrivers();
  if (n == 0) {
    SDL_Log("No built-in video drivers\n");
  } else {
    (void)SDL_snprintf(text, sizeof(text), "Built-in video drivers:");
    for (i = 0; i < n; ++i) {
      if (i > 0) {
        SDL_snprintfcat(text, sizeof(text), ",");
      }
      SDL_snprintfcat(text, sizeof(text), " %s", SDL_GetVideoDriver(i));
    }
    SDL_Log("%s\n", text);
  }

  SDL_Log("Video driver: %s\n", SDL_GetCurrentVideoDriver());

  n = SDL_GetNumRenderDrivers();
  if (n == 0) {
    SDL_Log("No built-in render drivers\n");
  } else {
    SDL_snprintf(text, sizeof(text), "Built-in render drivers:\n");
    for (i = 0; i < n; ++i) {
      SDL_snprintfcat(text, sizeof(text), "  %s\n", SDL_GetRenderDriver(i));
    }
    SDL_Log("%s\n", text);
  }

  SDL_DisplayID dispID = SDL_GetPrimaryDisplay();
  if (dispID == 0) {
    SDL_Log("No primary display found\n");
  } else {
    SDL_snprintf(text, sizeof(text), "Primary display info:\n");
    SDL_snprintfcat(text, sizeof(text), "  Name:\t\t\t%s\n", SDL_GetDisplayName(dispID));
    const SDL_DisplayMode* dispMode = SDL_GetCurrentDisplayMode(dispID);
    SDL_snprintfcat(text, sizeof(text), "  Pixel Format:\t\t%x\n", dispMode->format);
    SDL_snprintfcat(text, sizeof(text), "  Width:\t\t%d\n", dispMode->w);
    SDL_snprintfcat(text, sizeof(text), "  Height:\t\t%d\n", dispMode->h);
    SDL_snprintfcat(text, sizeof(text), "  Pixel Density:\t%f\n", dispMode->pixel_density);
    SDL_snprintfcat(text, sizeof(text), "  Refresh Rate:\t\t%f\n", dispMode->refresh_rate);
    SDL_Log("%s\n", text);
  }
}

void WindowManager_Init(void) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    wm_log.error_f("Couldn't initialize video driver: {}", SDL_GetError());
    return;
  }

  WindowManager::instance().create_sdl_window();

  PrintDebugInfo();

  TTF_Init();

  init_fonts();
}

WindowPtr WindowManager_CreateNewWindow(int16_t res_id, bool is_dialog, WindowPtr behind) {
  Rect bounds;
  int16_t proc_id;
  std::string title;
  bool visible;
  bool go_away;
  uint32_t ref_con;
  size_t num_dialog_items;
  std::vector<std::shared_ptr<DialogItem>> dialog_items;

  if (is_dialog) {
    auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_DLOG, res_id);
    auto dlog = ResourceDASM::ResourceFile::decode_DLOG(*data_handle, GetHandleSize(data_handle));
    bounds.left = dlog.bounds.x1;
    bounds.right = dlog.bounds.x2;
    bounds.top = dlog.bounds.y1;
    bounds.bottom = dlog.bounds.y2;
    proc_id = dlog.proc_id;
    title = dlog.title;
    visible = dlog.visible;
    go_away = dlog.go_away;
    ref_con = dlog.ref_con;
    dialog_items = DialogItem::from_DITL(dlog.items_id);

  } else {
    auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_WIND, res_id);
    auto wind = ResourceDASM::ResourceFile::decode_WIND(*data_handle, GetHandleSize(data_handle));
    bounds.left = wind.bounds.x1;
    bounds.right = wind.bounds.x2;
    bounds.top = wind.bounds.y1;
    bounds.bottom = wind.bounds.y2;
    proc_id = wind.proc_id;
    title = wind.title;
    visible = wind.visible;
    go_away = wind.go_away;
    ref_con = wind.ref_con;
  }

  return WindowManager::instance().create_window(
      title,
      bounds,
      visible,
      go_away,
      proc_id,
      ref_con,
      is_dialog,
      std::move(dialog_items));
}

void WindowManager_DrawDialog(WindowPtr theWindow) {
  WindowManager::instance().window_for_port(theWindow)->erase_and_render();
}

void WindowManager_DisposeWindow(WindowPtr theWindow) {
  if (theWindow == nullptr) {
    return;
  }
  WindowManager::instance().destroy_window(theWindow);
}

void GetDialogItem(DialogPtr dialog, short item_id, short* item_type, Handle* item_handle, Rect* box) {
  auto window = WindowManager::instance().window_for_port(reinterpret_cast<WindowPtr>(dialog));
  auto& items = window->get_dialog_items();

  try {
    auto item = items.at(item_id - 1);
    // Realmz doesn't use the handle directly; it only passes the handle to other
    // Classic Mac OS API functions. So, we can just return the DialogItem
    // opaque handle instead
    *item_type = macos_dialog_item_type_for_resource_dasm_type(item->type);
    *item_handle = wrap_opaque_handle(item->opaque_handle);
    *box = item->rect;
  } catch (const std::out_of_range&) {
    wm_log.warning_f("GetDialogItem called with invalid item_id {} (there are only {} items)", item_id, items.size());
  }
}

void GetDialogItemText(DialogItemHandle item_handle, Str255 text) {
  size_t handle = unwrap_opaque_handle(item_handle);
  auto item = DialogItem::get_item_by_handle(handle);
  pstr_for_string<256>(text, item->get_text());
}

void SetDialogItemText(DialogItemHandle item_handle, ConstStr255Param text) {
  size_t handle = unwrap_opaque_handle(item_handle);
  auto item = DialogItem::get_item_by_handle(handle);
  auto str = string_for_pstr<256>(text);
  wm_log.debug_f("SetDialogItemText({}, \"{}\")", item->str(), str);

  item->set_text(str);

  auto window = item->owner_window.lock();
  if (window) {
    item->render_in_port(window->get_port());
    WindowManager::instance().recomposite_from_window(window);
  } else {
    wm_log.warning_f("SetDialogItemText({}, \"{}\") cannot re-render window because owner_window is not set", item->str(), str);
  }
}

int16_t StringWidth(ConstStr255Param s) {
  return s[0];
}

Boolean IsDialogEvent(const EventRecord* ev) {
  try {
    auto* port = CCGrafPort::as_port(ev->window_port);
    return WindowManager::instance().window_for_port(port)->is_dialog();
  } catch (const std::out_of_range&) {
    return false;
  }
}

Boolean DialogSelect(const EventRecord* ev, DialogPtr* dialog, short* item_hit) {
  // Inside Macintosh: Toolbox Essentials describes the behavior as such:
  // (from https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-428.html):
  // 1. In response to an activate or update event for the dialog box,
  //    DialogSelect activates or updates its window and returns FALSE.
  // 2. If a key-down event or an auto-key event occurs and there's an editable
  //    text item in the dialog box, DialogSelect uses TextEdit to handle text
  //    entry and editing, and DialogSelect returns TRUE for a function result.
  //    In its itemHit parameter, DialogSelect returns the item number.
  // 3. If a key-down event or an auto-key event occurs and there's no editable
  //    text item in the dialog box, DialogSelect returns FALSE.
  // 4. If the user presses the mouse button while the cursor is in an editable
  //    text item, DialogSelect responds to the mouse activity as appropriate;
  //    that is, either by displaying an insertion point or by selecting text.
  //    If the editable text item is disabled, DialogSelect returns FALSE. If
  //    the editable text item is enabled, DialogSelect returns TRUE and in its
  //    itemHit parameter returns the item number. Normally, editable text
  //    items are disabled, and you use the GetDialogItemText function to read
  //    the information in the items only after the OK button is clicked.
  // 5. If the user presses the mouse button while the cursor is in a control,
  //    DialogSelect calls the Control Manager function TrackControl. If the
  //    user releases the mouse button while the cursor is in an enabled
  //    control, DialogSelect returns TRUE for a function result and in its
  //    itemHit parameter returns the control's item number. Your application
  //    should respond appropriately--for example, by performing a command
  //    after the user clicks the OK button.
  // 6. If the user presses the mouse button while the cursor is in any other
  //    enabled item in the dialog box, DialogSelect returns TRUE for a
  //    function result and in its itemHit parameter returns the item's number.
  //    Generally, only controls should be enabled. If your application creates
  //    a complex control, such as one that measures how far a dial is moved,
  //    your application must handle mouse events in that item before passing
  //    the event to DialogSelect.
  // 7. If the user presses the mouse button while the cursor is in a disabled
  //    item, or if it is in no item, or if any other event occurs,
  //    DialogSelect does nothing.
  // 8. If the event isn't one that DialogSelect specifically checks for (if
  //    it's a null event, for example), and if there's an editable text item
  //    in the dialog box, DialogSelect calls the TextEdit procedure TEIdle to
  //    make the insertion point blink.

  // The above is a lot of logic! Fortunately, we don't have to implement some
  // of it. (1) is not necessary because SDL handles activeness and updates,
  // and we hide all that from Realmz. (2) is not implemented yet but will
  // likely also be handled by SDL, so for key-down events we can just always
  // return false, which takes care of (3). We may have to implement (4) to
  // activate SDL edit controls when the user clicks them (TODO). We may also
  // have to implement (5) later on. (6) is implemented; Realmz uses it for a
  // lot of interactions. (7) and (8) don't do anything, so they're technically
  // implemented as well.

  auto window = WindowManager::instance().window_for_port(CCGrafPort::as_port(ev->window_port));

  // Before any of the expected logic, we implement a debugging feature: the
  // backslash key prints information about the dialog item that the user is
  // hovering over to stderr.
  if ((ev->what == keyDown) && ((ev->message & 0xFF) == static_cast<uint8_t>('\\'))) {
    try {
      const auto& items = window->get_dialog_items();
      fprintf(stderr, "Dialog items at (%hu, %hu):\n", ev->where.h, ev->where.v);
      for (size_t z = 0; z < items.size(); z++) {
        const auto item = items.at(z);
        if (PtInRect(ev->where, &item->rect)) {
          auto s = item->str();
          std::string processed_text_str = phosg::format_data_string(replace_param_text(item->get_text()));
          fprintf(stderr, "%s (processed_text=%s)\n", s.c_str(), processed_text_str.c_str());
        }
      }
    } catch (const std::out_of_range&) {
    }
  }

  // Backspace
  if (ev->what == keyDown && (mac_vk_from_message(ev->message) == MAC_VK_BACKSPACE)) {
    auto item = window->get_focused_item();
    if (!item) {
      return false;
    }
    window->delete_char(item);
  }

  // Handle cases (2) and (3) above. These would normally be emitted as keyDown events, but SDL distinguishes
  // key downs that are part of text input as distinct event types. See EventManager::enqueue_sdl_event().
  if (ev->what == app4Evt) {
    try {
      // Text input always happens in the currently focused item
      auto item = window->get_focused_item();

      // Case (3)
      if (!item) {
        return false;
      } else {
        // Here is where the Classic OS would intercept key down events that took place in an editable
        // text field and delegate processing to TextEdit. Since SDL provides dedicated event types for
        // text editing, we can do the same.
        window->handle_text_input(ev->text, item);
      }
    } catch (const std::out_of_range&) {
    }
  }

  // Handle case (6) described above
  if (ev->what == mouseDown) {
    try {
      auto item = window->dialog_item_for_position(ev->where, true);
      if (item) {
        *item_hit = item->item_id;

        // Currently, only editable text fields can be focused on, for text input
        if (item->type == DialogItemType::EDIT_TEXT) {
          window->set_focused_item(item);
          WindowManager::instance().on_dialog_item_focus_changed();
        }

        return true;
      }
    } catch (const std::out_of_range&) {
    }
  }
  return false;
}

void SystemClick(const EventRecord* theEvent, WindowPtr theWindow) {
  // This is used for handling events in windows belonging to the system, other
  // applications, or desk accessories. On modern systems we never see these
  // events, so we can just do nothing here.
}

void MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, Boolean front) {
  if (theWindow == nullptr) {
    return;
  }

  auto window = WindowManager::instance().window_for_port(theWindow);
  window->move(hGlobal, vGlobal);
}

void ShowWindow(WindowPtr theWindow) {
  if (theWindow == nullptr) {
    return;
  }

  auto window = WindowManager::instance().window_for_port(theWindow);
  window->show();
}

void SizeWindow(CWindowPtr theWindow, uint16_t w, uint16_t h, Boolean fUpdate) {
  auto window = WindowManager::instance().window_for_port(theWindow);
  window->resize(w, h);
}

DialogPtr GetNewDialog(uint16_t res_id, void* dStorage, WindowPtr behind) {
  return WindowManager_CreateNewWindow(res_id, true, behind);
}

CWindowPtr GetNewCWindow(int16_t res_id, void* wStorage, WindowPtr behind) {
  return WindowManager_CreateNewWindow(res_id, false, behind);
}

void DisposeDialog(DialogPtr theDialog) {
  WindowManager_DisposeWindow(theDialog);
}

void DrawDialog(DialogPtr theDialog) {
  WindowManager_DrawDialog(theDialog);
}

void NumToString(int32_t num, Str255 str) {
  str[0] = snprintf(reinterpret_cast<char*>(&str[1]), 0xFF, "%" PRId32, num);
}

void StringToNum(ConstStr255Param str, int32_t* num) {
  // Inside Macintosh I-490:
  //   StringToNum doesn't actually check whether the characters in the string
  //   are between '0' and '9'; instead, since the ASCII codes for '0' through
  //   '9' are $30 through $39, it just masks off the last four bits and uses
  //   them as a digit. For example, '2:' is converted to the number 30 because
  //   the ASCII code for':' is $3A. Spaces are treated as zeroes, since the
  //   ASCII code for a space is $20.
  // We implement the same behavior here.
  *num = 0;
  if (str[0] > 0) {
    bool negative = (str[1] == '-');
    size_t offset = negative ? 1 : 0;
    for (size_t z = 0; z < static_cast<uint8_t>(str[0]); z++) {
      *num = ((*num) * 10) + (str[z + 1] & 0x0F);
    }
    if (negative) {
      *num = -(*num);
    }
  }
}

void ModalDialog(ModalFilterProcPtr filterProc, short* itemHit) {

  // Retrieve the current window to only process events within that window
  CGrafPtr port = qd.thePort;

  // Skip all events until we get one that's within the current window
  // (condition 1) and not handled as part of the dialog abstraction
  // (conditions 2 & 3). DialogSelect will fill in `item`, which we then return
  EventRecord e;
  DialogPtr dialog;
  short item;
  do {
    WaitNextEvent(everyEvent, &e, 1, NULL);
  } while (e.window_port != port || !IsDialogEvent(&e) || !DialogSelect(&e, &dialog, &item));

  *itemHit = item;
}

ControlHandle GetNewControl(int16_t cntl_id, WindowPtr window) {
  auto w = WindowManager::instance().window_for_port(window);
  auto control = Control::from_CNTL(cntl_id);
  auto item = DialogItem::from_control(control);
  w->add_dialog_item(item);
  return wrap_opaque_handle(item->opaque_handle);
}

ControlHandle NewControl(
    WindowPtr window,
    const Rect* bounds,
    ConstStr255Param title,
    Boolean visible,
    short value,
    short min,
    short max,
    short proc_id,
    long ref_con) {
  auto w = WindowManager::instance().window_for_port(window);
  auto title_str = string_for_pstr<256>(title);
  auto control = Control::from_params(*bounds, value, min, max, proc_id, visible, title_str);
  auto item = DialogItem::from_control(control);
  w->add_dialog_item(item);
  return wrap_opaque_handle(item->opaque_handle);
}

static void render_window_for_item(std::shared_ptr<DialogItem> item) {
  auto window = item->owner_window.lock();
  if (window) {
    item->render_in_port(window->get_port());
    WindowManager::instance().recomposite_from_window(window);
  }
}

static void set_control_visible(ControlHandle handle, bool visible) {
  wm_log.debug_f("set_control_visible({}, {})", reinterpret_cast<void*>(handle), visible);
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  if (item->set_control_visible(visible)) {
    render_window_for_item(item);
  }
}

void ShowControl(ControlHandle handle) {
  set_control_visible(handle, true);
}
void HideControl(ControlHandle handle) {
  set_control_visible(handle, false);
}

void GetControlBounds(ControlHandle handle, Rect* rect) {
  if (!handle) {
    return;
  }
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  if (item->control) {
    *rect = item->control->bounds;
  } else {
    // This probably actually resulted in undefined behavior on original MacOS
    rect->left = 0;
    rect->top = 0;
    rect->right = 0;
    rect->bottom = 0;
  }
}

void MoveControl(ControlHandle handle, short h, short v) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  wm_log.debug_f("MoveControl({}, {}, {})", item->str(), h, v);
  if (item->move_control(h, v)) {
    render_window_for_item(item);
  }
}

void SizeControl(ControlHandle handle, short w, short h) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  wm_log.debug_f("SizeControl({}, {}, {})", item->str(), w, h);
  if (item->resize_control(w, h)) {
    render_window_for_item(item);
  }
}

void DrawControls(WindowPtr port) {
  wm_log.debug_f("DrawControls({})", reinterpret_cast<void*>(port));

  // TODO: Does this suffice? Do we need to also set the dirty flag for all
  // controls or something like that?
  auto window = WindowManager::instance().window_for_port(port);
  // TODO: Should we draw all items, or only controls?
  for (const auto& item : window->get_dialog_items()) {
    if (item->control) {
      item->render_in_port(window->get_port());
    }
  }
  WindowManager::instance().recomposite_from_window(window);
}

short FindControl(Point pt, WindowPtr window, ControlHandle* handle) {
  auto w = WindowManager::instance().window_for_port(window);
  for (const auto& item : w->get_dialog_items()) {
    if (item->control && PtInRect(pt, &item->control->bounds)) {
      *handle = wrap_opaque_handle(item->opaque_handle);
      switch (item->control->type) {
        case ControlType::BUTTON:
        case ControlType::WINDOW_FONT_BUTTON:
          return kControlButtonPart;
        case ControlType::CHECKBOX:
        case ControlType::WINDOW_FONT_CHECKBOX:
          return kControlCheckBoxPart;
        case ControlType::RADIO_BUTTON:
        case ControlType::WINDOW_FONT_RADIO_BUTTON:
          return kControlRadioButtonPart;
        case ControlType::SCROLL_BAR:
          // TODO: We should implement the up/down buttons at some point
          return kControlIndicatorPart;
        case ControlType::POPUP_MENU:
          return kControlMenuPart;
        case ControlType::UNKNOWN:
        default:
          throw std::logic_error("Unknown control type");
      }
    }
  }
  *handle = wrap_opaque_handle(0);
  return 0;
}

short TrackControl(ControlHandle handle, Point pt, ProcPtr action_proc) {
  // TODO: This should do something! It seems this is only used for the scroll
  // bars in the shop screen, so I assume we can get to this later.
  return 0;
}

short GetControlValue(ControlHandle handle) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  return item->control ? item->control->value : 0;
}

short GetControlMinimum(ControlHandle handle) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  return item->control ? item->control->min : 0;
}

short GetControlMaximum(ControlHandle handle) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  return item->control ? item->control->max : 0;
}

void SetControlValue(ControlHandle handle, short value) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  wm_log.debug_f("SetControlValue({}, {})", item->str(), value);
  if (item->set_control_value(value)) {
    render_window_for_item(item);
  }
}

void SetControlMinimum(ControlHandle handle, short min) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  wm_log.debug_f("SetControlMinimum({}, {})", item->str(), min);
  if (item->set_control_minimum(min)) {
    render_window_for_item(item);
  }
}

void SetControlMaximum(ControlHandle handle, short max) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  wm_log.debug_f("SetControlMaximum({}, {})", item->str(), max);
  if (item->set_control_maximum(max)) {
    render_window_for_item(item);
  }
}

void GetControlTitle(ControlHandle handle, Str255 title) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  if (item->control) {
    pstr_for_string<256>(title, item->control->title);
  }
}

WindowPtr FrontWindow() {
  auto window = WindowManager::instance().front_window();
  return window ? &window->get_port() : nullptr;
}

void BringToFront(WindowPtr theWindow) {
  auto& wm = WindowManager::instance();
  auto window = wm.window_for_port(theWindow);
  wm.bring_to_front(window);
}

void DisposeWindow(WindowPtr theWindow) {
  WindowManager_DisposeWindow(theWindow);
}

TEHandle TENew(const Rect* destRect, const Rect* viewRect) {
  auto window = WindowManager::instance().window_for_port(qd.thePort);
  return window->add_text_edit(*destRect, *viewRect);
}

void TESetText(const void* text, int32_t length, TEHandle hTE) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(hTE));
  item->set_text(string_for_pstr<256>(static_cast<const unsigned char*>(const_cast<void*>(text))));
}

void TESetSelect(int32_t selStart, int32_t selEnd, TEHandle hTE) {
  // It looks like Realmz only uses this once, and sets the start and end to 0 and 1,
  // respectively, effectively a no-op.
}

void TEUpdate(const Rect* r, TEHandle hTE) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(hTE));
  wm_log.debug_f("TEUpdate({{x0={}, y0={}, x1={}, y1={}}}, {})", r->left, r->top, r->right, r->bottom, reinterpret_cast<void*>(hTE));
  item->set_dirty_flag(); // Force re-render of DialogItem
  auto window = item->owner_window.lock();
  item->render_in_port(window->get_port());
  WindowManager::instance().recomposite_from_window(window);
}

void TEDispose(TEHandle hTE) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(hTE));
  auto window = item->owner_window.lock();
  window->remove_text_edit(item);
}
