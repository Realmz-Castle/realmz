#include "WindowManager.hpp"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_properties.h>
#include <memory>
#include <stdexcept>
#include <variant>
#include <vector>

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
#include "GraphicsCanvas.hpp"
#include "MemoryManager.h"
#include "QuickDraw.hpp"
#include "ResourceManager.h"
#include "StringConvert.hpp"

using ResourceDASM::ResourceFile;

class WindowManager;
class Window;

inline size_t unwrap_opaque_handle(Handle h) {
  static_assert(sizeof(size_t) == sizeof(Handle));
  return reinterpret_cast<size_t>(h);
}
inline Handle wrap_opaque_handle(size_t h) {
  static_assert(sizeof(size_t) == sizeof(Handle));
  return reinterpret_cast<Handle>(h);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// SDL and rendering helpers

static phosg::PrefixedLogger wm_log("[WindowManager] ");

static size_t generate_opaque_handle() {
  static size_t next_handle = 1;
  return next_handle++;
}

typedef std::variant<TTF_Font*, ResourceDASM::BitmapFontRenderer> Font;

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
        throw std::runtime_error(phosg::string_printf("Unknown control type %hd", proc_id));
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
    Rect bounds;
    copy_rect(bounds, def.bounds);
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
    return phosg::string_printf(
        "Control(cntl_resource_id=%hd, opaque_handle=%zu, type=%s, value=%hd, min=%hd, max=%hd, visible=%s)",
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

  // Source information
  int32_t ditl_resource_id; // 0x00010000 = not from a DITL
  size_t item_id;

  // Options
  DialogItemType type;
  int16_t resource_id; // From item definition (generally used for controls)
  Rect rect;
  bool enabled;
  std::shared_ptr<Control> control; // May be null

  // Window and renderer
  std::weak_ptr<Window> window;
  GraphicsCanvas canvas;
  sdl_window_shared sdl_window;

  static std::unordered_map<size_t, std::weak_ptr<DialogItem>> all_items;

  static std::shared_ptr<DialogItem> get_item_by_handle(size_t handle) {
    auto item = DialogItem::all_items.at(handle).lock();
    if (!item) {
      throw std::logic_error(phosg::string_printf(
          "Attempted to get missing or destroyed dialog item (handle was %zu)", handle));
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
    return phosg::string_printf(
        "DialogItem(ditl_resource_id=%" PRId32 ", item_id=%zu, type=%s, resource_id=%hd, rect=Rect(left=%hd, top=%hd, right=%hd, bottom=%hd), enabled=%s, control=%s, handle=%zu, dirty=%s, text=%s)",
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

  bool init(CGrafPtr port) {
    canvas = GraphicsCanvas(sdl_window, rect, port);
    return canvas.init();
  }

  // Draw the dialog item contents to a local texture, so that the dialog item
  // can preserve its rendered state to be recomposited in subsequent window render
  // calls
  void update() {
    canvas.clear();

    CGrafPtr port = qd.thePort;

    switch (type) {
      case ResourceFile::DecodedDialogItem::Type::PICTURE: {
        const auto& pict = **GetPicture(resource_id);
        const Rect& r = pict.picFrame;
        int16_t w = r.right - r.left;
        int16_t h = r.bottom - r.top;
        // Since we're drawing to the local texture buffer, we want to fill it, rather than draw
        // to the bounds specified by the resource.
        canvas.draw_rgba_picture(*pict.data, w, h, Rect{0, 0, h, w});
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::ICON:
        // TODO
        wm_log.warning("Attempted to draw ICON dialog item, but it's not implemented");
        break;
      case ResourceFile::DecodedDialogItem::Type::TEXT: {
        if (text.length() < 1) {
          dirty = false;
          return;
        }
        if (!canvas.draw_text(
                text,
                Rect{0, 0, get_height(), get_width()})) {
          wm_log.error("Error when rendering text item %d: %s", resource_id, SDL_GetError());
          dirty = false;
          return;
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::BUTTON: {
        if (!canvas.draw_text(
                text,
                Rect{0, 0, get_height(), get_width()})) {
          wm_log.error("Error when rendering button text item %d: %s", resource_id, SDL_GetError());
          dirty = false;
          return;
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::EDIT_TEXT: {
        if (!canvas.draw_text(
                text,
                Rect{0, 0, get_height(), get_width()})) {
          wm_log.error("Error when rendering editable text item %d: %s", resource_id, SDL_GetError());
          dirty = false;
          return;
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::CHECKBOX:
      case ResourceFile::DecodedDialogItem::Type::RADIO_BUTTON:
        // TODO: For now, we just draw radio buttons the same as checkboxes. (Does Realmz even use radio buttons?)
        // Draw checkbox
        canvas.set_draw_color(port->rgbFgColor);
        canvas.draw_line(Point{.h = 0, .v = 0}, Point{.h = 0, .v = 10}); // Left side
        canvas.draw_line(Point{.h = 0, .v = 0}, Point{.h = 10, .v = 0}); // Top side
        canvas.draw_line(Point{.h = 10, .v = 10}, Point{.h = 0, .v = 10}); // Bottom side
        canvas.draw_line(Point{.h = 10, .v = 10}, Point{.h = 10, .v = 0}); // Right side
        if (control && control->value) {
          // Draw an X also if the checkbox is checked
          canvas.draw_line(Point{.h = 0, .v = 0}, Point{.h = 10, .v = 10});
          canvas.draw_line(Point{.h = 0, .v = 10}, Point{.h = 10, .v = 0});
        }
        if (!canvas.draw_text(
                text,
                Rect{12, 0, get_height(), get_width()})) {
          wm_log.error("Error when rendering button text item %d: %s", resource_id, SDL_GetError());
          dirty = false;
          return;
        }
        break;
      case ResourceFile::DecodedDialogItem::Type::RESOURCE_CONTROL:
      case ResourceFile::DecodedDialogItem::Type::HELP_BALLOON:
      case ResourceFile::DecodedDialogItem::Type::CUSTOM:
      case ResourceFile::DecodedDialogItem::Type::UNKNOWN:
        // TODO: Should we draw anything for these types?
        break;
      default:
        break;
    }

    dirty = false;
  }

  // Render the DialogItem's current texture to the window.
  void render() {
    if (dirty) {
      update();
    }

    SDL_FRect dstRect;
    dstRect.x = rect.left;
    dstRect.y = rect.top;
    dstRect.w = get_width();
    dstRect.h = get_height();

    canvas.render(&dstRect);
  }

  int16_t get_width() const {
    return rect.right - rect.left;
  }

  int16_t get_height() const {
    return rect.bottom - rect.top;
  }

  const std::string& get_text() const {
    return text;
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

  bool move_control(short h, short v) {
    if (this->control) {
      Rect& bounds = this->control->bounds;
      int32_t w = bounds.right - bounds.left;
      int32_t h = bounds.bottom - bounds.top;
      bounds.left = h;
      bounds.top = v;
      bounds.right = bounds.left + w;
      bounds.bottom = bounds.top + h;
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

class Window : public std::enable_shared_from_this<Window> {
private:
  std::string title;
  Rect bounds;
  int w;
  int h;
  CWindowRecord cWindowRecord;
  sdl_window_shared sdl_window;
  std::shared_ptr<GraphicsCanvas> canvas;
  bool is_dialog_flag;
  std::vector<std::shared_ptr<DialogItem>> dialog_items; // All items (the below 3 vectors are disjoint subsets of this)
  std::vector<std::shared_ptr<DialogItem>> static_items;
  std::vector<std::shared_ptr<DialogItem>> control_items;
  std::vector<std::shared_ptr<DialogItem>> text_items;
  std::shared_ptr<DialogItem> focused_item;
  bool text_editing_active;

public:
  Window() = default;
  Window(
      std::string title,
      const Rect& bounds,
      CWindowRecord record,
      bool is_dialog,
      std::vector<std::shared_ptr<DialogItem>>&& dialog_items)
      : title{title},
        bounds{bounds},
        cWindowRecord{record},
        is_dialog_flag{is_dialog},
        dialog_items{std::move(dialog_items)},
        canvas{} {
    w = bounds.right - bounds.left;
    h = bounds.bottom - bounds.top;
    focused_item = nullptr;
  }

  ~Window() {
    deregister_canvas(canvas);
  }

  void init() {
    SDL_WindowFlags flags{};

    if (cWindowRecord.windowKind == plainDBox) {
      flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY;
    }
    if (!cWindowRecord.visible) {
      flags |= SDL_WINDOW_HIDDEN;
    }
    sdl_window = sdl_make_shared(SDL_CreateWindow(title.c_str(), w, h, flags));

    if (sdl_window == NULL) {
      throw std::runtime_error(phosg::string_printf("Could not create window: %s\n", SDL_GetError()));
    }

    canvas = std::make_shared<GraphicsCanvas>(sdl_window, bounds, get_port());
    register_canvas(canvas);
    canvas->init();

    for (auto di : this->dialog_items) {
      di->window = this->weak_from_this();
      di->sdl_window = sdl_window;
      di->init(get_port());

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

      if (di->type == DialogItemType::EDIT_TEXT && !text_editing_active) {
        SDL_Rect r{
            di->rect.left,
            di->rect.top,
            di->rect.right - di->rect.left,
            di->rect.bottom - di->rect.top};
        init_text_editing(r);
      }
    }

    canvas->clear();
  }

  void init_text_editing(SDL_Rect r) {
    // Macintosh Toolbox Essentials 6-32
    if (!SDL_SetTextInputArea(sdl_window.get(), &r, 0)) {
      wm_log.error("Could not create text area: %s", SDL_GetError());
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);
    SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN, false);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);

    if (!SDL_StartTextInputWithProperties(sdl_window.get(), props)) {
      wm_log.error("Could not start text input: %s", SDL_GetError());
    }

    SDL_DestroyProperties(props);

    text_editing_active = true;
  }

  void add_dialog_item(std::shared_ptr<DialogItem> item) {
    item->item_id = this->dialog_items.size();
    this->dialog_items.emplace_back(item);
  }

  std::shared_ptr<DialogItem> get_focused_item() {
    return focused_item;
  }

  CGrafPtr get_port() {
    return &cWindowRecord.port;
  }

  void set_focused_item(std::shared_ptr<DialogItem> item) {
    if (item->window.lock()->sdl_window == sdl_window) {
      focused_item = item;
    }
  }

  void handle_text_input(const std::string& text, std::shared_ptr<DialogItem> item) {
    item->append_text(text);
    render(true);
  }

  void delete_char(std::shared_ptr<DialogItem> item) {
    item->delete_char();
    render(true);
  }

  void render(bool renderDialogItems = true) {
    if (!cWindowRecord.visible) {
      return;
    }

    // Clear the backbuffer before drawing frame
    canvas->clear_window();

    CGrafPtr port = qd.thePort;
    if (port->bkPixPat) {
      canvas->draw_background(sdl_window, port->bkPixPat);
    }

    // The DrawDialog procedure draws the entire contents of the specified dialog box. The
    // DrawDialog procedure draws all dialog items, calls the Control Manager procedure
    // DrawControls to draw all controls, and calls the TextEdit procedure TEUpdate to
    // update all static and editable text items and to draw their display rectangles. The
    // DrawDialog procedure also calls the application-defined items’ draw procedures if
    // the items’ rectangles are within the update region.
    if (renderDialogItems) {
      for (auto item : this->static_items) {
        item->render();
      }
      for (auto item : this->control_items) {
        item->render();
      }
      for (auto item : this->text_items) {
        item->render();
      }
    }

    canvas->render(NULL);

    // Flush changes to screen
    canvas->sync();
  }

  void move(int hGlobal, int vGlobal) {
    SDL_SetWindowPosition(sdl_window.get(), hGlobal, vGlobal);
    SDL_SyncWindow(sdl_window.get());
  }

  void resize(uint16_t w, uint16_t h) {
    auto& portRect = cWindowRecord.port.portRect;
    portRect.right = portRect.left + w;
    portRect.bottom = portRect.top + h;

    this->w = w;
    this->h = h;

    if (SDL_SetWindowSize(sdl_window.get(), w, h)) {
      canvas->sync();
    } else {
      wm_log.error("Could not resize window: %s", SDL_GetError());
    }
  }

  void show() {
    cWindowRecord.visible = true;
    render(false);
    SDL_ShowWindow(sdl_window.get());
  }

  SDL_WindowID sdl_window_id() const {
    return SDL_GetWindowID(sdl_window.get());
  }

  const std::vector<std::shared_ptr<DialogItem>>& get_dialog_items() const {
    return this->dialog_items;
  }

  std::shared_ptr<DialogItem> dialog_item_for_position(const Point& pt, bool enabled_only) {
    for (const auto& item : this->dialog_items) {
      if ((!enabled_only || item->enabled) && PtInRect(pt, &item->rect)) {
        return item;
      }
    }
    return nullptr;
  }

  inline bool is_dialog() const {
    return this->is_dialog_flag;
  }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Window manager

class WindowManager {
private:
  std::unordered_map<DialogItemHandle, std::shared_ptr<DialogItem>> dialog_items_by_handle;

public:
  WindowManager() = default;
  ~WindowManager() = default;

  WindowPtr create_window(
      const std::string& title,
      const Rect& bounds,
      bool visible,
      bool go_away,
      int16_t proc_id,
      uint32_t ref_con,
      bool is_dialog,
      std::vector<std::shared_ptr<DialogItem>>&& dialog_items) {
    CGrafPtr current_port = qd.thePort;

    CGrafPort port{};
    port.portRect = bounds;

    CWindowRecord* wr = new CWindowRecord();
    wr->port = port;
    wr->port.portRect = bounds;
    wr->port.txFont = current_port->txFont;
    wr->port.txFace = current_port->txFace;
    wr->port.txMode = current_port->txMode;
    wr->port.txSize = current_port->txSize;

    wr->port.fgColor = current_port->fgColor;
    wr->port.bgColor = current_port->bgColor;
    wr->port.rgbFgColor = current_port->rgbFgColor;
    wr->port.rgbBgColor = current_port->rgbBgColor;

    // Note: Realmz doesn't actually use any of the following fields; we also
    // don't use numItems and dItems internally here (we instead use the vector
    // in the Window struct)
    wr->visible = visible;
    wr->goAwayFlag = go_away;
    wr->windowKind = proc_id;
    wr->refCon = ref_con;

    auto window = std::make_shared<Window>(title, bounds, *wr, is_dialog, std::move(dialog_items));

    // Must call init here to create SDL resources and associate the window with its DialogItems
    window->init();
    record_to_window.emplace(window->get_port(), window);
    sdl_window_id_to_window.emplace(window->sdl_window_id(), window);

    if (wr->visible) {
      window->render(false);
    }

    // Maintain a shared lookup across all windows of their dialog items, by handle,
    // to support functions that modify the DITLs directly, like SetDialogItemText
    for (auto di : dialog_items) {
      DialogItem::all_items[di->opaque_handle] = di;
    }

    return window->get_port();
  }

  void destroy_window(WindowPtr record) {
    auto window_it = record_to_window.find(record);
    if (window_it == record_to_window.end()) {
      throw std::logic_error("Attempted to delete nonexistent window");
    }

    sdl_window_id_to_window.erase(window_it->second->sdl_window_id());
    record_to_window.erase(window_it);

    // If the current port is this window's port, set the current port back to
    // the default port
    CGrafPtr current_port = qd.thePort;
    if (current_port == record) {
      SetPort(&qd.defaultPort);
    }
  }

  std::shared_ptr<Window> window_for_record(WindowPtr record) {
    return this->record_to_window.at(record);
  }
  std::shared_ptr<Window> window_for_sdl_window_id(SDL_WindowID window_id) {
    return this->sdl_window_id_to_window.at(window_id);
  }

  std::shared_ptr<DialogItem> dialog_item_for_handle(DialogItemHandle handle) {
    return dialog_items_by_handle.at(handle);
  }

private:
  std::unordered_map<WindowPtr, std::shared_ptr<Window>> record_to_window;
  std::unordered_map<SDL_WindowID, std::shared_ptr<Window>> sdl_window_id_to_window;
};

static WindowManager wm;

void render_window(CGrafPtr record) {
  try {
    auto window = wm.window_for_record(record);
    window->render();
  } catch (std::out_of_range) {
    // This is probably an offscreen GWorld with software renderer and no window
    if (lookup_canvas(record)->is_window()) {
      throw std::runtime_error("Tried to render a window from its port, but it doesn't exist in lookup");
    }
  }
}

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Classic Mac OS API

void WindowManager_Init(void) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    wm_log.error("Couldn't initialize video driver: %s\n", SDL_GetError());
    return;
  }

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

  return wm.create_window(
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
  CWindowRecord* const windowRecord = reinterpret_cast<CWindowRecord*>(theWindow);
  auto window = wm.window_for_record(theWindow);

  window->render(true);
}

void WindowManager_DisposeWindow(WindowPtr theWindow) {
  if (theWindow == nullptr) {
    return;
  }

  wm.destroy_window(theWindow);
}

DisplayProperties WindowManager_GetPrimaryDisplayProperties(void) {
  auto displayID = SDL_GetPrimaryDisplay();

  if (displayID == 0) {
    wm_log.error("Could not get primary display: %s", SDL_GetError());
    return {};
  }

  SDL_Rect bounds{};
  if (!SDL_GetDisplayBounds(displayID, &bounds)) {
    wm_log.error("Could not get display bounds: %s", SDL_GetError());
    return {};
  }

  return DisplayProperties{
      bounds.w,
      bounds.h};
}

void GetDialogItem(DialogPtr dialog, short item_id, short* item_type, Handle* item_handle, Rect* box) {
  auto window = wm.window_for_record(reinterpret_cast<WindowPtr>(dialog));
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
    wm_log.warning("GetDialogItem called with invalid item_id %hd (there are only %zu items)", item_id, items.size());
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
  item->set_text(string_for_pstr<256>(text));
  auto window = item->window.lock();
  window->render(true);
}

int16_t StringWidth(ConstStr255Param s) {
  return s[0];
}

Boolean IsDialogEvent(const EventRecord* ev) {
  try {
    return wm.window_for_sdl_window_id(ev->sdl_window_id)->is_dialog();
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

  // Before any of the expected logic, we implement a debugging feature: the
  // backslash key prints information about the dialog item that the user is
  // hovering over to stderr.
  if ((ev->what == keyDown) && ((ev->message & 0xFF) == static_cast<uint8_t>('\\'))) {
    try {
      auto window = wm.window_for_sdl_window_id(ev->sdl_window_id);
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
    auto window = wm.window_for_sdl_window_id(ev->sdl_window_id);

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
      auto window = wm.window_for_sdl_window_id(ev->sdl_window_id);

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
      auto window = wm.window_for_sdl_window_id(ev->sdl_window_id);
      auto item = window->dialog_item_for_position(ev->where, true);
      if (item) {
        *item_hit = item->item_id;

        // Currently, only editable text fields can be focused on, for text input
        if (item->type == DialogItemType::EDIT_TEXT) {
          window->set_focused_item(item);
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

  auto window = wm.window_for_record(theWindow);
  window->move(hGlobal, vGlobal);
}

void ShowWindow(WindowPtr theWindow) {
  if (theWindow == nullptr) {
    return;
  }

  auto window = wm.window_for_record(theWindow);
  window->show();
}

void SizeWindow(CWindowPtr theWindow, uint16_t w, uint16_t h, Boolean fUpdate) {
  auto window = wm.window_for_record(theWindow);
  // Hack: Don't resize the window if it's the main window (not a dialog). This
  // is because SDL automatically centers windows, and we don't want the window
  // to be full-screen anyway.
  if (window->is_dialog()) {
    window->resize(w, h);
  }
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
  EventRecord e;
  DialogPtr dialog;
  short item;

  // Retrieve the current window to only process events within that window
  CGrafPtr port = qd.thePort;

  do {
    WaitNextEvent(everyEvent, &e, 1, NULL);
  } while (
      e.sdl_window_id != wm.window_for_record(port)->sdl_window_id() ||
      !IsDialogEvent(&e) ||
      !DialogSelect(&e, &dialog, &item));

  *itemHit = item;
}

ControlHandle GetNewControl(int16_t cntl_id, WindowPtr window) {
  auto w = wm.window_for_record(window);
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
  auto w = wm.window_for_record(window);
  auto title_str = string_for_pstr<256>(title);
  auto control = Control::from_params(*bounds, value, min, max, proc_id, visible, title_str);
  auto item = DialogItem::from_control(control);
  w->add_dialog_item(item);
  return wrap_opaque_handle(item->opaque_handle);
}

static void render_window_for_item(std::shared_ptr<DialogItem> item) {
  auto window = item->window.lock();
  if (window) {
    window->render();
  }
}

static void set_control_visible(ControlHandle handle, bool visible) {
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
  if (item->move_control(h, v)) {
    render_window_for_item(item);
  }
}

void SizeControl(ControlHandle handle, short w, short h) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  if (item->resize_control(w, h)) {
    render_window_for_item(item);
  }
}

void DrawControls(WindowPtr window) {
  // TODO: Does this suffice? Do we need to also set the dirty flag for all
  // controls or something like that?
  auto w = wm.window_for_record(window);
  w->render();
}

short FindControl(Point pt, WindowPtr window, ControlHandle* handle) {
  auto w = wm.window_for_record(window);
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
  if (item->set_control_value(value)) {
    render_window_for_item(item);
  }
}

void SetControlMinimum(ControlHandle handle, short min) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
  if (item->set_control_minimum(min)) {
    render_window_for_item(item);
  }
}

void SetControlMaximum(ControlHandle handle, short max) {
  auto item = DialogItem::get_item_by_handle(unwrap_opaque_handle(handle));
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
