#include "WindowManager.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_properties.h>
#include <memory>
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
#include "FileManager.hpp"
#include "MemoryManager.h"
#include "QuickDraw.hpp"
#include "ResourceManager.h"
#include "StringConvert.hpp"

#define BLACK_CHANCERY_FONT_ID 1602
#define GENEVA_FONT_ID 0
#define CHICAGO_FONT_ID 1

using ResourceDASM::ResourceFile;

class DialogItem;

static phosg::PrefixedLogger wm_log("[WindowManager] ");
static std::unordered_map<int16_t, TTF_Font*> tt_fonts_by_id;
static std::unordered_map<int16_t, ResourceDASM::BitmapFontRenderer> bm_renderers_by_id;

std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> sdl_make_unique(SDL_Surface* s) {
  return std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(s, SDL_DestroySurface);
}
std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)> sdl_make_unique(SDL_Texture* t) {
  return std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)>(t, SDL_DestroyTexture);
}

typedef std::variant<TTF_Font*, ResourceDASM::BitmapFontRenderer> Font;
typedef size_t DialogItemHandle;
static std::unordered_map<DialogItemHandle, std::shared_ptr<DialogItem>> dialog_items_by_opaque_handle;

using DialogItemType = ResourceDASM::ResourceFile::DecodedDialogItem::Type;

std::array<std::string, 4> param_text_entries;

static std::string replace_param_text(const std::string& text) {
  char prev = 0;
  std::string ret;
  for (size_t z = 0; z < text.size(); z++) {
    char ch = text[z];
    if (ch == '^' && z < text.size() - 1) {
      char index = text[++z];
      if (index == '0' && !param_text_entries[0].empty()) {
        ret += param_text_entries[0];
      } else if (index == '1' && !param_text_entries[1].empty()) {
        ret += param_text_entries[1];
      } else if (index == '2' && !param_text_entries[2].empty()) {
        ret += param_text_entries[2];
      } else if (index == '3' && !param_text_entries[3].empty()) {
        ret += param_text_entries[3];
      } else {
        ret.append(1, '^');
        ret.append(1, (index == '\r') ? '\n' : index);
      }
    } else {
      ret.append(1, (ch == '\r') ? '\n' : ch);
    }
  }
  return ret;
}

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

void debug_renderer_state(SDL_Renderer* renderer, const std::string& label) {
  SDL_Surface* surface = SDL_RenderReadPixels(renderer, NULL);
  if (surface) {
    IMG_SavePNG(surface, label.c_str());
  }
}

void copy_rect(Rect& dst, const ResourceDASM::Rect& src) {
  dst.top = src.y1;
  dst.left = src.x1;
  dst.bottom = src.y2;
  dst.right = src.x2;
}

bool render_surface(std::shared_ptr<SDL_Renderer> sdlRenderer, SDL_Surface* surface, const Rect& rect) {
  SDL_FRect text_dest;
  text_dest.x = rect.left;
  text_dest.y = rect.top;
  text_dest.w = std::min<float>(rect.right - rect.left, surface->w);
  text_dest.h = std::min<float>(rect.bottom - rect.top, surface->h);
  auto texture = SDL_CreateTextureFromSurface(sdlRenderer.get(), surface);

  if (!texture) {
    wm_log.error("Could not render surface: %s", SDL_GetError());
    return false;
  }

  auto text_texture = sdl_make_unique(texture);

  // DEBUG: Uncomment for debugging rendered surfaces
  if (text_texture) {
    // IMG_SavePNG(surface, "render_surface.png");
  }

  if (!text_texture) {
    wm_log.error("Failed to create texture when rendering text: %s", SDL_GetError());
    return false;
  } else if (!SDL_RenderTexture(sdlRenderer.get(), text_texture.get(), NULL, &text_dest)) {
    wm_log.error("Failed to render text texture: %s", SDL_GetError());
    return false;
  }
  return true;
}

void draw_rgba_picture(std::shared_ptr<SDL_Renderer> sdlRenderer, void* pixels, int w, int h, const Rect& rect) {
  auto s = sdl_make_unique(SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, 4 * w));
  if (!s) {
    wm_log.error("Could not create surface: %s\n", SDL_GetError());
    return;
  }
  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(sdlRenderer.get(), s.get()));
  if (!t) {
    wm_log.error("Could not create texture: %s\n", SDL_GetError());
    return;
  }
  SDL_FRect dest;
  dest.x = rect.left;
  dest.y = rect.top;
  dest.w = rect.right - rect.left;
  dest.h = rect.bottom - rect.top;
  SDL_RenderTexture(sdlRenderer.get(), t.get(), NULL, &dest);
}

void draw_rgba_picture(std::shared_ptr<SDL_Renderer> sdlRenderer, void* pixels, int w, int h, const ResourceDASM::Rect& dispRect) {
  Rect bounds{};
  copy_rect(bounds, dispRect);
  draw_rgba_picture(sdlRenderer, pixels, w, h, bounds);
}

bool draw_text_ttf(std::shared_ptr<SDL_Renderer> sdlRenderer, TTF_Font* font, const std::string& processed_text, const Rect& rect) {
  auto text_surface = sdl_make_unique(TTF_RenderText_Blended_Wrapped(
      font, processed_text.data(), processed_text.size(), GetForeColorSDL(), rect.right - rect.left));
  if (!text_surface) {
    wm_log.error("Failed to create surface when rendering text: %s", SDL_GetError());
    return false;
  } else {
    return render_surface(sdlRenderer, text_surface.get(), rect);
  }
}

bool draw_text_bitmap(
    std::shared_ptr<SDL_Renderer> sdlRenderer,
    const ResourceDASM::BitmapFontRenderer& renderer,
    const std::string& processed_text,
    const Rect& rect) {
  size_t rect_w = rect.right - rect.left;
  size_t rect_h = rect.bottom - rect.top;
  phosg::Image img(rect_w, rect_h, true);
  img.clear(0xFF00FF00); // Transparent (with magenta so it will be obvious if compositing is done incorrectly)

  std::string wrapped_text = renderer.wrap_text_to_pixel_width(processed_text, rect_w);
  renderer.render_text(img, wrapped_text, 0, 0, GetForeColorRGBA8888());

  // DEBUG: Uncomment for debugging text rendering
  // img.save("bitmap_text.png", phosg::Image::Format::PNG);

  auto text_surface = sdl_make_unique(SDL_CreateSurfaceFrom(
      rect_w, rect_h, SDL_PIXELFORMAT_RGBA32, img.get_data(), 4 * img.get_width()));
  if (!text_surface) {
    wm_log.error("Failed to create surface when rendering text: %s", SDL_GetError());
    return false;
  } else {
    return render_surface(sdlRenderer, text_surface.get(), rect);
  }
}

// Tries to load a TrueType font first; if it's not available, use a
// bitmapped font instead.
bool load_font(int16_t font_id, Font& font) {
  try {
    font = tt_fonts_by_id.at(font_id);
    return true;
  } catch (const std::out_of_range&) {
  }

  try {
    font = bm_renderers_by_id.at(font_id);
    return true;
  } catch (const std::out_of_range&) {
    auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_FONT, font_id);
    auto decoded = std::make_shared<ResourceDASM::ResourceFile::DecodedFontResource>(
        ResourceDASM::ResourceFile::decode_FONT_only(*data_handle, GetHandleSize(data_handle)));
    font = bm_renderers_by_id.emplace(font_id, decoded).first->second;
  }

  return true;
}

bool draw_text(std::shared_ptr<SDL_Renderer> sdlRenderer, const std::string& text, const Rect& dispRect, int16_t font_id) {
  std::string processed_text = replace_param_text(text);

  Font font{};

  bool success = load_font(font_id, font);

  if (!success) {
    return false;
  }

  if (std::holds_alternative<TTF_Font*>(font)) {
    return draw_text_ttf(sdlRenderer, std::get<TTF_Font*>(font), processed_text, dispRect);
  } else {
    return draw_text_bitmap(sdlRenderer, std::get<ResourceDASM::BitmapFontRenderer>(font), processed_text, dispRect);
  }

  wm_log.error("No renderer is available for font %hd; cannot render text \"%s\"", font_id, text.c_str());
  return false;
}

// Draws the specified text when the display bounds are unknown. Returns the width of
// the rendered text in pixels.
int draw_text(std::shared_ptr<SDL_Renderer> sdlRenderer, const std::string& text, int16_t x, int16_t y, int16_t font_id) {
  std::string processed_text = replace_param_text(text);

  Font font{};

  bool success = load_font(font_id, font);

  if (!success) {
    return -1;
  }

  if (std::holds_alternative<TTF_Font*>(font)) {
    TTF_Font* tt_font = std::get<TTF_Font*>(font);
    TTF_Text* t = TTF_CreateText(NULL, tt_font, processed_text.c_str(), 0);
    int w{0}, h{0};
    TTF_GetTextSize(t, &w, &h);

    // The pen location, passed in as the x and y parameters, is at the baseline of the text, to
    // the left. So, we need to account for this in our display rect.
    Rect r{static_cast<int16_t>(y - h), x, static_cast<int16_t>(y + h), static_cast<int16_t>(x + w)};
    TTF_DestroyText(t);
    if (!draw_text_ttf(sdlRenderer, tt_font, processed_text, r)) {
      return -1;
    }
    return w;
  } else {
    // TODO: Implement text measuring for bitmap fonts
    wm_log.error("Rendering bitmap text with unknown dimensions is not supported");
    return -1;
  }

  wm_log.error("No renderer is available for font %hd; cannot render text \"%s\"", font_id, text.c_str());
  return -1;
}

class WindowManager;
class Window;

// This structure is "private" (not accessible in C) because it isn't directly
// used there: Realmz only interacts with dialog items through syscalls and
// handles, so we can use C++ types here without breaking anything.
struct DialogItem {
public:
  int16_t ditl_resource_id;
  size_t item_id;
  DialogItemType type;
  int16_t resource_id;
  Rect rect;
  bool enabled;
  std::weak_ptr<Window> window;
  std::shared_ptr<SDL_Renderer> sdlRenderer;
  std::shared_ptr<SDL_Window> sdlWindow;
  DialogItemHandle handle;
  static DialogItemHandle next_di_handle;
  static size_t next_item_id;
  static std::unordered_map<size_t, std::shared_ptr<DialogItem>> all_dialog_items;

private:
  bool dirty;
  SDL_Texture* sdlTexture;
  std::string text;

public:
  DialogItem(int16_t ditl_res_id, size_t item_id, const ResourceDASM::ResourceFile::DecodedDialogItem& def)
      : ditl_resource_id(ditl_res_id),
        item_id(item_id),
        type(def.type),
        text(def.text),
        resource_id(def.resource_id),
        enabled(def.enabled),
        handle{next_di_handle++},
        dirty{true},
        sdlTexture{nullptr} {
    this->rect.left = def.bounds.x1;
    this->rect.right = def.bounds.x2;
    this->rect.top = def.bounds.y1;
    this->rect.bottom = def.bounds.y2;
  }

  static std::vector<std::shared_ptr<DialogItem>> from_DITL(int16_t ditl_resource_id) {
    auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_DITL, ditl_resource_id);
    auto defs = ResourceDASM::ResourceFile::decode_DITL(*data_handle, GetHandleSize(data_handle));

    std::vector<std::shared_ptr<DialogItem>> ret;
    for (const auto& decoded_dialog_item : defs) {
      size_t item_id = ret.size() + 1;
      auto di = ret.emplace_back(new DialogItem(ditl_resource_id, item_id, decoded_dialog_item));
      dialog_items_by_opaque_handle.insert({di->handle, di});
    }
    return ret;
  }

  ~DialogItem() {
    if (sdlTexture) {
      SDL_DestroyTexture(sdlTexture);
    }
    dialog_items_by_opaque_handle.erase(handle);
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
    return phosg::string_printf(
        "DialogItem(ditl_resource_id=%hd, item_id=%zu, type=%s, resource_id=%hd, rect=Rect(left=%hd, top=%hd, right=%hd, bottom=%hd), enabled=%s, handle=%zu, dirty=%s, text=%s)",
        this->ditl_resource_id,
        this->item_id,
        type_strs.at(this->type),
        this->resource_id,
        this->rect.left,
        this->rect.top,
        this->rect.right,
        this->rect.bottom,
        this->enabled ? "true" : "false",
        this->handle,
        this->dirty ? "true" : "false",
        text_str.c_str());
  }

  void update() {
    if (!sdlTexture) {
      sdlTexture = SDL_CreateTexture(sdlRenderer.get(), SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET, get_width(), get_height());

      if (!sdlTexture) {
        throw std::runtime_error("Could not create dialog item texture");
      }
    }

    // Draw the dialog item contents to a local texture, so that the dialog item
    // can preserve its rendered state to be recomposited in subsequent window render
    // calls
    SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);

    // Clear the texture with transparent background pixels, using no blend mode
    SDL_BlendMode blendMode;
    uint8_t r, g, b, a;
    SDL_GetRenderDrawBlendMode(sdlRenderer.get(), &blendMode);
    SDL_GetRenderDrawColor(sdlRenderer.get(), &r, &b, &g, &a);
    SDL_SetRenderDrawBlendMode(sdlRenderer.get(), SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(sdlRenderer.get(), 0, 0, 0, SDL_ALPHA_TRANSPARENT);
    SDL_RenderClear(sdlRenderer.get());
    SDL_SetRenderDrawBlendMode(sdlRenderer.get(), blendMode);
    SDL_SetRenderDrawColor(sdlRenderer.get(), r, g, b, a);

    CGrafPtr port;
    GetPort(&port);

    switch (type) {
      case ResourceFile::DecodedDialogItem::Type::PICTURE: {
        const auto& pict = **GetPicture(resource_id);
        const Rect& r = pict.picFrame;
        int16_t w = r.right - r.left;
        int16_t h = r.bottom - r.top;
        // Since we're drawing to the local texture buffer, we want to fill it, rather than draw
        // to the bounds specified by the resource.
        draw_rgba_picture(sdlRenderer, *pict.data, w, h, Rect{0, 0, h, w});
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::TEXT: {
        if (text.length() < 1) {
          SDL_SetRenderTarget(sdlRenderer.get(), NULL);
          dirty = false;
          return;
        }
        if (!draw_text(
                sdlRenderer,
                text,
                Rect{0, 0, get_height(), get_width()},
                port->txFont)) {
          wm_log.error("Error when rendering text item %d: %s", resource_id, SDL_GetError());
          SDL_SetRenderTarget(sdlRenderer.get(), NULL);
          dirty = false;
          return;
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::BUTTON: {
        if (!draw_text(
                sdlRenderer,
                text,
                Rect{0, 0, get_height(), get_width()},
                port->txFont)) {
          wm_log.error("Error when rendering button text item %d: %s", resource_id, SDL_GetError());
          SDL_SetRenderTarget(sdlRenderer.get(), NULL);
          dirty = false;
          return;
        }
        break;
      }
      case ResourceFile::DecodedDialogItem::Type::EDIT_TEXT: {
        if (!draw_text(
                sdlRenderer,
                text,
                Rect{0, 0, get_height(), get_width()},
                port->txFont)) {
          wm_log.error("Error when rendering editable text item %d: %s", resource_id, SDL_GetError());
          SDL_SetRenderTarget(sdlRenderer.get(), NULL);
          dirty = false;
          return;
        }
        break;
      }
      default:
        // TODO: Render other DITL types
        break;
    }

    SDL_SetRenderTarget(sdlRenderer.get(), NULL);
    dirty = false;
  }

  // Render the DialogItem's current texture to the current rendering target.
  void render() {
    if (dirty) {
      update();
    }

    SDL_FRect dstRect;
    dstRect.x = rect.left;
    dstRect.y = rect.top;
    dstRect.w = get_width();
    dstRect.h = get_height();
    SDL_RenderTexture(sdlRenderer.get(), sdlTexture, NULL, &dstRect);
  }

  int16_t get_width() const {
    return rect.right - rect.left;
  }

  int16_t get_height() const {
    return rect.bottom - rect.top;
  }

  const std::string& get_text() {
    return text;
  }

  void set_text(const std::string& new_text) {
    text = new_text;
    dirty = true;
  }

  void set_text(std::string&& new_text) {
    text = std::move(new_text);
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
};

// Initialize static handle sequence
DialogItemHandle DialogItem::next_di_handle = 1;

class Window : public std::enable_shared_from_this<Window> {
public:
  Window() = default;
  Window(std::string title, const Rect& bounds, CWindowRecord record, std::shared_ptr<std::vector<std::shared_ptr<DialogItem>>> dialog_items)
      : title{title},
        bounds{bounds},
        cWindowRecord{record},
        dialogItems{dialog_items} {
    w = bounds.right - bounds.left;
    h = bounds.bottom - bounds.top;
    focusedItem = nullptr;
  }

  void init() {
    SDL_WindowFlags flags{};

    if (cWindowRecord.windowKind == plainDBox) {
      flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY;
    }
    if (!cWindowRecord.visible) {
      flags |= SDL_WINDOW_HIDDEN;
    }
    std::shared_ptr<SDL_Window> wind(SDL_CreateWindow(title.c_str(), w, h, flags), SDL_DestroyWindow);
    sdlWindow = wind.get();

    if (sdlWindow == NULL) {
      throw std::runtime_error(phosg::string_printf("Could not create window: %s\n", SDL_GetError()));
    }

    std::shared_ptr<SDL_Renderer> r(SDL_CreateRenderer(sdlWindow, "opengl"), SDL_DestroyRenderer);
    sdlRenderer = r;

    if (sdlRenderer == NULL) {
      throw std::runtime_error(phosg::string_printf("Could not create window renderer: %s\n", SDL_GetError()));
    }

    // We'll use this texture as our own backbuffer, see
    // https://stackoverflow.com/questions/63759688/sdl-renderpresent-implementation
    // The SDL wiki also shows this technique of drawing to an in-memory texture buffer
    // before rendering to the backbuffer: https://wiki.libsdl.org/SDL3/SDL_CreateTexture
    sdlTexture = SDL_CreateTexture(sdlRenderer.get(), SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_TARGET, w, h);

    if (sdlTexture == NULL) {
      throw std::runtime_error(phosg::string_printf("Could not create window texture: %s\n", SDL_GetError()));
    }

    if (dialogItems) {
      for (auto di : *dialogItems) {
        di->window = this->weak_from_this();
        di->sdlRenderer = sdlRenderer;
        di->sdlWindow = wind;

        // Set the focused text field to be the first EDIT_TEXT item encountered
        if (!focusedItem && di->type == DialogItemType::EDIT_TEXT) {
          focusedItem = di;
        }

        if (di->type == DialogItemType::TEXT || di->type == DialogItemType::EDIT_TEXT) {
          textItems.emplace_back(di);
        } else {
          renderableItems.emplace_back(di);
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
    }

    SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);
    SDL_RenderClear(sdlRenderer.get());
    SDL_SetRenderTarget(sdlRenderer.get(), NULL);
  }

  void init_text_editing(SDL_Rect r) {
    // Macintosh Toolbox Essentials 6-32
    if (!SDL_SetTextInputArea(sdlWindow, &r, 0)) {
      wm_log.error("Could not create text area: %s", SDL_GetError());
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);
    SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN, false);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);

    if (!SDL_StartTextInputWithProperties(sdlWindow, props)) {
      wm_log.error("Could not start text input: %s", SDL_GetError());
    }

    SDL_DestroyProperties(props);

    text_editing_active = true;
  }

  ~Window() {
    SDL_DestroyTexture(this->sdlTexture);
    SDL_DestroyWindow(this->sdlWindow);
  }

  bool draw_rect(const Rect& dispRect) {
    SDL_Surface* s = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);

    if (s == NULL) {
      wm_log.error("Could not create surface: %s\n", SDL_GetError());
      return false;
    }

    SDL_Texture* t = SDL_CreateTextureFromSurface(sdlRenderer.get(), s);
    if (t == NULL) {
      wm_log.error("Could not create texture: %s\n", SDL_GetError());
      return false;
    }
    SDL_DestroySurface(s);

    SDL_FRect dest;
    dest.x = dispRect.left;
    dest.y = dispRect.top;
    dest.w = dispRect.right - dispRect.left;
    dest.h = dispRect.bottom - dispRect.top;

    SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);
    SDL_RenderTexture(sdlRenderer.get(), t, NULL, &dest);
    SDL_SetRenderTarget(sdlRenderer.get(), NULL);
    return true;
  }

  void draw_rgba_picture(void* pixels, int w, int h, const ResourceDASM::Rect& dispRect) {
    SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);
    ::draw_rgba_picture(sdlRenderer, pixels, w, h, dispRect);
    SDL_SetRenderTarget(sdlRenderer.get(), NULL);
  }

  void draw_rgba_picture(void* pixels, int w, int h, const Rect& dispRect) {
    SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);
    ::draw_rgba_picture(sdlRenderer, pixels, w, h, dispRect);
    SDL_SetRenderTarget(sdlRenderer.get(), NULL);
  }

  void draw_line(const Point& start, const Point& end, const RGBColor& color) {
    SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);

    SDL_SetRenderDrawColor(sdlRenderer.get(), color.red, color.green, color.blue, SDL_ALPHA_OPAQUE);
    SDL_RenderLine(
        sdlRenderer.get(),
        static_cast<float>(start.h),
        static_cast<float>(start.v),
        static_cast<float>(end.h),
        static_cast<float>(end.v));

    // Restore window as the render target
    SDL_SetRenderTarget(sdlRenderer.get(), NULL);
  }

  // Draws the specified text at the current pen location and with the current
  // font characteristics stored in the window's port
  void draw_text(const std::string& text) {
    CGrafPtr port{};
    GetPort(&port);
    auto pen_loc = port->pnLoc;
    auto font_id = port->txFont;

    SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);

    int width = ::draw_text(sdlRenderer, text, pen_loc.h, pen_loc.v, font_id);
    port->pnLoc.h += width;

    // Restore window as the render target
    SDL_SetRenderTarget(sdlRenderer.get(), NULL);
  }

  std::shared_ptr<DialogItem> get_focused_item() {
    return focusedItem;
  }

  void set_focused_item(std::shared_ptr<DialogItem> item) {
    if (item->window.lock()->sdlWindow == sdlWindow) {
      focusedItem = item;
    }
  }

  void handle_text_input(const std::string& text, std::shared_ptr<DialogItem> item) {
    item->append_text(text);
    render();
  }

  void delete_char(std::shared_ptr<DialogItem> item) {
    item->delete_char();
    render();
  }

  void sync() {
    SDL_RenderPresent(sdlRenderer.get());
    SDL_SyncWindow(sdlWindow);
  }

  void render_background(PixPatHandle bkPixPat) {
    PixMapHandle pmap = (*bkPixPat)->patMap;
    Rect bounds = (*pmap)->bounds;
    int w = bounds.right - bounds.left;
    int h = bounds.bottom - bounds.top;
    SDL_Surface* s = SDL_CreateSurfaceFrom(
        w,
        h,
        SDL_PIXELFORMAT_RGB24,
        (*(*bkPixPat)->patData),
        3 * w);

    if (s == NULL) {
      wm_log.error("Could not create surface: %s\n", SDL_GetError());
      return;
    }

    SDL_Texture* t = SDL_CreateTextureFromSurface(sdlRenderer.get(), s);
    if (t == NULL) {
      wm_log.error("Could not create texture: %s\n", SDL_GetError());
      return;
    }
    SDL_DestroySurface(s);
    if (!SDL_RenderTextureTiled(sdlRenderer.get(), t, NULL, 1.0, NULL)) {
      wm_log.error("Could not render background texture: %s", SDL_GetError());
    }
  }

  void render() {
    if (!cWindowRecord.visible) {
      return;
    }

    // Clear the backbuffer before drawing frame
    uint8_t r, g, b, a;
    SDL_GetRenderDrawColor(sdlRenderer.get(), &r, &g, &b, &a);
    SDL_SetRenderDrawColor(sdlRenderer.get(), 0, 0, 0, 0);
    SDL_RenderClear(sdlRenderer.get());
    SDL_SetRenderDrawColor(sdlRenderer.get(), r, g, b, a);

    // DEBUG
    // debug_renderer_state(sdlRenderer.get(), "window.png");

    if (SDL_GetRenderTarget(sdlRenderer.get())) {
      throw std::logic_error("Tried to render window, but non-default target set");
    }

    CGrafPtr port;
    GetPort(&port);
    if (port->bkPixPat) {
      render_background(port->bkPixPat);
      // DEBUG
      // debug_renderer_state(sdlRenderer.get(), "window.png");
    }

    // The DrawDialog procedure draws the entire contents of the specified dialog box. The
    // DrawDialog procedure draws all dialog items, calls the Control Manager procedure
    // DrawControls to draw all controls, and calls the TextEdit procedure TEUpdate to
    // update all static and editable text items and to draw their display rectangles. The
    // DrawDialog procedure also calls the application-defined items’ draw procedures if
    // the items’ rectangles are within the update region.
    if (dialogItems) {
      for (auto item : renderableItems) {
        item->render();
        // DEBUG
        // debug_renderer_state(sdlRenderer.get(), "window.png");
      }

      for (auto item : textItems) {
        item->render();
      }
    }

    SDL_RenderTexture(sdlRenderer.get(), sdlTexture, NULL, NULL);
    // DEBUG
    // debug_renderer_state(sdlRenderer.get(), "window.png");

    // Flush changes to screen
    sync();
  }

  void move(int hGlobal, int vGlobal) {
    SDL_SetWindowPosition(sdlWindow, hGlobal, vGlobal);
    SDL_SyncWindow(sdlWindow);
  }

  void resize(uint16_t w, uint16_t h) {
    auto& portRect = cWindowRecord.port.portRect;
    portRect.right = portRect.left + w;
    portRect.bottom = portRect.top + h;

    this->w = w;
    this->h = h;

    if (SDL_SetWindowSize(sdlWindow, w, h)) {
      sync();
    } else {
      wm_log.error("Could not resize window: %s", SDL_GetError());
    }
  }

  void show() {
    cWindowRecord.visible = true;
    render();
    SDL_ShowWindow(sdlWindow);
  }

  SDL_WindowID sdl_window_id() const {
    return SDL_GetWindowID(this->sdlWindow);
  }

  SDL_Window* sdl_window() const {
    return sdlWindow;
  }

  std::shared_ptr<std::vector<std::shared_ptr<DialogItem>>> get_dialog_items() const {
    return this->dialogItems;
  }

  std::shared_ptr<DialogItem> dialog_item_for_position(const Point& pt, bool enabled_only) {
    auto items = this->get_dialog_items();
    if (items) {
      for (size_t z = 0; z < items->size(); z++) {
        const auto item = items->at(z);
        if ((!enabled_only || item->enabled) && PtInRect(pt, &item->rect)) {
          return item;
        }
      }
    }
    return nullptr;
  }

private:
  std::string title;
  Rect bounds;
  int w;
  int h;
  CWindowRecord cWindowRecord;
  SDL_Window* sdlWindow;
  std::shared_ptr<SDL_Renderer> sdlRenderer;
  std::shared_ptr<std::vector<std::shared_ptr<DialogItem>>> dialogItems;
  std::vector<std::shared_ptr<DialogItem>> renderableItems;
  std::vector<std::shared_ptr<DialogItem>> textItems;
  std::shared_ptr<DialogItem> focusedItem;
  SDL_Texture* sdlTexture; // Use a texture to hold the window's rendered base canvas
  bool text_editing_active;
};

class WindowManager {
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
      std::shared_ptr<std::vector<std::shared_ptr<DialogItem>>> dialog_items) {
    CGrafPtr current_port;
    GetPort(reinterpret_cast<GrafPtr*>(&current_port));

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

    std::shared_ptr<Window> window = std::make_shared<Window>(title, bounds, *wr, dialog_items);

    // Must call init here to create SDL resources and associate the window with its DialogItems
    window->init();
    record_to_window.emplace(&wr->port, window);
    sdl_window_id_to_window.emplace(window->sdl_window_id(), window);

    window->render();

    return &wr->port;
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
    CGrafPtr current_port;
    GetPort(reinterpret_cast<GrafPtr*>(&current_port));
    if (current_port == record) {
      SetPort(get_default_quickdraw_port());
    }

    CWindowRecord* const window = reinterpret_cast<CWindowRecord*>(record);
    delete window;
  }

  std::shared_ptr<Window> window_for_record(WindowPtr record) {
    return this->record_to_window.at(record);
  }
  std::shared_ptr<Window> window_for_sdl_window_id(SDL_WindowID window_id) {
    return this->sdl_window_id_to_window.at(window_id);
  }

private:
  std::unordered_map<WindowPtr, std::shared_ptr<Window>> record_to_window;
  std::unordered_map<SDL_WindowID, std::shared_ptr<Window>> sdl_window_id_to_window;
};

static WindowManager wm;

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
    wm_log.error("Couldn't initialize video driver: %s\n", SDL_GetError());
    return;
  }

  PrintDebugInfo();

  TTF_Init();

  // Pre-populate Black Chancery, since it doesnt' come from the resource fork.
  // We can't pre-populate Theldrow because the resource files aren't loaded at
  // the time WindowManager_Init is called.

  // TODO: Is 1602 the correct font ID for Black Chancery?
  auto font_filename = host_filename_for_mac_filename(":Black Chancery.ttf", true);
  tt_fonts_by_id[BLACK_CHANCERY_FONT_ID] = TTF_OpenFont(font_filename.c_str(), 16);
  font_filename = host_filename_for_mac_filename(":Geneva.ttf", true);
  tt_fonts_by_id[GENEVA_FONT_ID] = TTF_OpenFont(font_filename.c_str(), 16);
  font_filename = host_filename_for_mac_filename(":Chicago.ttf", true);
  tt_fonts_by_id[CHICAGO_FONT_ID] = TTF_OpenFont(font_filename.c_str(), 16);
}

WindowPtr WindowManager_CreateNewWindow(int16_t res_id, bool is_dialog, WindowPtr behind) {
  Rect bounds;
  int16_t proc_id;
  std::string title;
  bool visible;
  bool go_away;
  uint32_t ref_con;
  size_t num_dialog_items;
  std::shared_ptr<std::vector<std::shared_ptr<DialogItem>>> dialog_items;

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
    dialog_items = make_shared<std::vector<std::shared_ptr<DialogItem>>>(DialogItem::from_DITL(dlog.items_id));

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
      dialog_items);
}

void WindowManager_DrawDialog(WindowPtr theWindow) {
  CWindowRecord* const windowRecord = reinterpret_cast<CWindowRecord*>(theWindow);
  auto window = wm.window_for_record(theWindow);

  window->render();
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

OSErr PlotCIcon(const Rect* theRect, CIconHandle theIcon) {
  GrafPtr port;
  GetPort(&port);
  auto window = wm.window_for_record(reinterpret_cast<CGrafPort*>(port));
  auto bounds = (*theIcon)->iconBMap.bounds;
  int w = bounds.right - bounds.left;
  int h = bounds.bottom - bounds.top;
  window->draw_rgba_picture(
      *((*theIcon)->iconData),
      w, h, *theRect);
  window->render();

  return noErr;
}

void GetDialogItem(DialogPtr dialog, short item_id, short* item_type, Handle* item_handle, Rect* box) {
  auto window = wm.window_for_record(reinterpret_cast<WindowPtr>(dialog));
  auto items = window->get_dialog_items();
  if (!items) {
    throw std::logic_error("GetDialogItem called on non-dialog window");
  }

  try {
    auto item = items->at(item_id - 1);
    // Realmz doesn't use the handle directly; it only passes the handle to other
    // Classic Mac OS API functions. So, we can just return the DialogItem
    // opaque handle instead
    *item_type = macos_dialog_item_type_for_resource_dasm_type(item->type);
    *item_handle = reinterpret_cast<Handle>(item->handle);
    *box = item->rect;
  } catch (const std::out_of_range&) {
    wm_log.warning("GetDialogItem called with invalid item_id %hd (there are only %zu items)", item_id, items->size());
  }
}

void GetDialogItemText(Handle item_handle, Str255 text) {
  // See comment in GetDialogItem about why this isn't a real Handle
  auto handle = reinterpret_cast<DialogItemHandle>(item_handle);
  auto item = dialog_items_by_opaque_handle.at(handle);
  pstr_for_string<256>(text, item->get_text());
}

void SetDialogItemText(Handle item_handle, ConstStr255Param text) {
  // See comment in GetDialogItem about why this isn't a real Handle
  auto handle = reinterpret_cast<DialogItemHandle>(item_handle);
  auto item = dialog_items_by_opaque_handle.at(handle);
  item->set_text(string_for_pstr<256>(text));
  auto window = item->window.lock();
  window->render();
}

int16_t StringWidth(ConstStr255Param s) {
  return s[0];
}

Boolean IsDialogEvent(const EventRecord* ev) {
  try {
    auto window = wm.window_for_sdl_window_id(ev->sdl_window_id);
    return (window->get_dialog_items() != nullptr);
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
      auto items = window->get_dialog_items();
      if (items) {
        fprintf(stderr, "Dialog items at (%hu, %hu):\n", ev->where.h, ev->where.v);
        for (size_t z = 0; z < items->size(); z++) {
          const auto item = items->at(z);
          if (PtInRect(ev->where, &item->rect)) {
            auto s = item->str();
            std::string processed_text_str = phosg::format_data_string(replace_param_text(item->get_text()));
            fprintf(stderr, "%s (processed_text=%s)\n", s.c_str(), processed_text_str.c_str());
          }
        }
      } else {
        fprintf(stderr, "Current window does not have dialog items\n");
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

void DrawPicture(PicHandle myPicture, const Rect* dstRect) {
  CGrafPtr port;
  GetPort(&port);

  try {
    auto window = wm.window_for_record(port);

    auto picFrame = (*myPicture)->picFrame;
    int w = picFrame.right - picFrame.left;
    int h = picFrame.bottom - picFrame.top;

    window->draw_rgba_picture(*((*myPicture)->data), w, h, *dstRect);
    window->render();
  } catch (std::out_of_range e) {
    wm_log.warning("Could not find window for current port");
    return;
  }
}

void LineTo(int16_t h, int16_t v) {
  CGrafPtr port;
  GetPort(&port);

  try {
    auto window = wm.window_for_record(port);

    window->draw_line(port->pnLoc, {v, h}, port->rgbBgColor);
    window->render();
    port->pnLoc = {v, h};
  } catch (std::out_of_range e) {
    wm_log.warning("Could not find window for current port");
    return;
  }
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
  // Hack: Don't resize the window if it's the main window. This is because SDL
  // automatically centers windows, and we don't want the window to be
  // full-screen anyway.
  if (window->get_dialog_items() != nullptr) {
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

void ParamText(ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3) {
  param_text_entries[0] = string_for_pstr<256>(param0);
  param_text_entries[1] = string_for_pstr<256>(param1);
  param_text_entries[2] = string_for_pstr<256>(param2);
  param_text_entries[3] = string_for_pstr<256>(param3);
  fprintf(stderr, "ParamText(\"%s\", \"%s\", \"%s\", \"%s\")\n", param_text_entries[0].c_str(), param_text_entries[1].c_str(), param_text_entries[2].c_str(), param_text_entries[3].c_str());
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
  CGrafPtr port;
  GetPort(&port);

  do {
    WaitNextEvent(everyEvent, &e, 1, NULL);
  } while (
      e.sdl_window_id != wm.window_for_record(port)->sdl_window_id() ||
      !IsDialogEvent(&e) ||
      !DialogSelect(&e, &dialog, &item));

  *itemHit = item;
}

void DrawString(ConstStr255Param s) {
  CGrafPtr port;
  GetPort(&port);

  auto window = wm.window_for_record(port);

  auto str = string_for_pstr<255>(s);

  window->draw_text(str);
  window->render();
}