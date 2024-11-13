#include "WindowManager.h"

#include "FileManager.hpp"
#include "RealmzCocoa.h"
#include "StringConvert.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <phosg/Strings.hh>

#include "MemoryManager.hpp"
#include "QuickDraw.hpp"

using ResourceDASM::ResourceFile;

static phosg::PrefixedLogger wm_log("[WindowManager] ");
static std::unordered_map<int16_t, TTF_Font*> fonts_by_id;

void copy_rect(Rect& dst, const ResourceDASM::Rect& src) {
  dst.top = src.y1;
  dst.left = src.x1;
  dst.bottom = src.y2;
  dst.right = src.x2;
}

// Macintosh Toolbox Essentials (6-121 Dialog Manager Reference)
short resource_type_to_int(ResourceDASM::ResourceFile::DecodedDialogItem::Type type) {
  switch (type) {
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::BUTTON:
      return 4;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::CHECKBOX:
      return 5;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::RADIO_BUTTON:
      return 6;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::RESOURCE_CONTROL:
      return 7;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::TEXT:
      return 8;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::EDIT_TEXT:
      return 16;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::ICON:
      return 32;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::PICTURE:
      return 64;
    case ResourceDASM::ResourceFile::DecodedDialogItem::Type::CUSTOM:
      return 0;
    default:
      wm_log.error("Unrecognized dialog item type");
      return -1;
  }
}

void draw_rgba_picture(std::shared_ptr<SDL_Renderer> sdlRenderer, void* pixels, int w, int h, const Rect& dispRect) {
  SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, 4 * w);

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

  SDL_FRect dest;
  dest.x = dispRect.left;
  dest.y = dispRect.top;
  dest.w = dispRect.right - dispRect.left;
  dest.h = dispRect.bottom - dispRect.top;

  SDL_RenderTexture(sdlRenderer.get(), t, NULL, &dest);
}
void draw_rgba_picture(std::shared_ptr<SDL_Renderer> sdlRenderer, void* pixels, int w, int h, const ResourceDASM::Rect& dispRect) {
  Rect bounds{};
  copy_rect(bounds, dispRect);
  draw_rgba_picture(sdlRenderer, pixels, w, h, bounds);
}

bool draw_text(std::shared_ptr<SDL_Renderer> sdlRenderer, const std::string& text, const Rect& dispRect) {
  // TTF_Font* font = fonts_by_id.at(port->txFont);
  TTF_Font* font = fonts_by_id.at(1601);
  RGBColor fore_color;
  int width = dispRect.right - dispRect.left;
  GetForeColor(&fore_color);
  SDL_Surface* text_surface = TTF_RenderText_Blended_Wrapped(
      font,
      reinterpret_cast<const char*>(text.c_str()),
      static_cast<size_t>(text.length()),
      SDL_Color{
          static_cast<uint8_t>(fore_color.red / 0x0101),
          static_cast<uint8_t>(fore_color.green / 0x0101),
          static_cast<uint8_t>(fore_color.blue / 0x0101),
          255},
      width);
  if (!text_surface) {
    return false;
  }
  SDL_FRect text_dest;
  text_dest.x = dispRect.left;
  text_dest.y = dispRect.top;
  text_dest.w = std::min(width, text_surface->w);
  text_dest.h = std::min(dispRect.bottom - dispRect.top, text_surface->h);
  SDL_Texture* text_texture = SDL_CreateTextureFromSurface(sdlRenderer.get(), text_surface);
  if (!text_texture || !SDL_RenderTexture(sdlRenderer.get(), text_texture, NULL, &text_dest)) {
    return false;
  }
  return true;
}
bool draw_text(std::shared_ptr<SDL_Renderer> sdlRenderer, const std::string& text, const ResourceDASM::Rect& dispRect) {
  Rect bounds{};
  copy_rect(bounds, dispRect);
  draw_text(sdlRenderer, text, bounds);
}

class WindowManager {
public:
  class DialogItem {
  public:
    DialogItemHandle dialogItemHandle;

    DialogItem()
        : sdlTexture{nullptr},
          dialogItemHandle{},
          sdlRenderer{std::shared_ptr<SDL_Renderer>()} {
    }

    DialogItem(const ResourceDASM::ResourceFile::DecodedDialogItem& di, std::shared_ptr<SDL_Renderer> r)
        : sdlRenderer{r} {

      auto dialogItemHandle = NewHandleTyped<ResourceDASM::ResourceFile::DecodedDialogItem>();
      **dialogItemHandle = di;
      sdlTexture = SDL_CreateTexture(sdlRenderer.get(), SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET, get_width(), get_height());
    }

    ~DialogItem() {
      SDL_DestroyTexture(sdlTexture);
    }

    void update() {
      SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);
      SDL_RenderClear(sdlRenderer.get());

      switch ((*dialogItemHandle)->type) {
        case ResourceFile::DecodedDialogItem::Type::PICTURE: {
          auto pict = **GetPicture((*dialogItemHandle)->resource_id);
          Rect r = pict.picFrame;
          uint32_t w = r.right - r.left;
          uint32_t h = r.bottom - r.top;
          draw_rgba_picture(sdlRenderer, *pict.data, w, h, (*dialogItemHandle)->bounds);
          break;
        }
        case ResourceFile::DecodedDialogItem::Type::TEXT: {
          if ((*dialogItemHandle)->text.length() < 1) {
            return;
          }
          if (!draw_text(sdlRenderer, (*dialogItemHandle)->text, (*dialogItemHandle)->bounds)) {
            wm_log.error("Error when rendering text item %d: %s", (*dialogItemHandle)->resource_id, SDL_GetError());
            return;
          }
          break;
        }
        case ResourceFile::DecodedDialogItem::Type::BUTTON: {
          if (!draw_text(sdlRenderer, (*dialogItemHandle)->text, (*dialogItemHandle)->bounds)) {
            wm_log.error("Error when rendering button text item %d: %s", (*dialogItemHandle)->resource_id, SDL_GetError());
            return;
          }
          break;
        }
        default:
          // TODO: Render other DITL types
          break;
      }

      // Restore window as the render target
      SDL_SetRenderTarget(sdlRenderer.get(), NULL);
    }

    // Render the DialogItem's current texture to the current rendering target.
    void render() {
      SDL_FRect dest;
      dest.x = (*dialogItemHandle)->bounds.x1;
      dest.y = (*dialogItemHandle)->bounds.y1;
      dest.w = get_width();
      dest.h = get_height();
      SDL_RenderTexture(sdlRenderer.get(), sdlTexture, NULL, &dest);
    }

    int get_width() {
      return (*dialogItemHandle)->bounds.width();
    }

    int get_height() {
      return (*dialogItemHandle)->bounds.height();
    }

  private:
    SDL_Texture* sdlTexture;
    std::shared_ptr<SDL_Renderer> sdlRenderer;
  };

  class Window {
  public:
    Window() = default;
    Window(
        std::string title,
        const Rect& bounds,
        SDL_WindowFlags flags,
        const std::vector<ResourceDASM::ResourceFile::DecodedDialogItem> dialog_items)
        : title{title},
          bounds{bounds},
          flags{flags},
          dialogItems{} {
      for (auto& di : dialog_items) {
        dialogItems.emplace_back(di);
      }
    }

    ~Window() {
      SDL_DestroyTexture(this->sdlTexture);
      SDL_DestroyWindow(this->sdlWindow);
    }

    void init() {
      w = bounds.right - bounds.left;
      h = bounds.bottom - bounds.top;
      sdlWindow = SDL_CreateWindow(
          title.c_str(),
          w,
          h,
          flags);

      if (sdlWindow == NULL) {
        wm_log.error("Could not create window: %s\n", SDL_GetError());
        return;
      }

      std::shared_ptr<SDL_Renderer> r(SDL_CreateRenderer(sdlWindow, "opengl"), [](auto p) {
        SDL_DestroyRenderer(p);
      });
      sdlRenderer = r;

      if (sdlRenderer == NULL) {
        wm_log.error("could not create renderer: %s\n", SDL_GetError());
        return;
      }

      // We'll use this texture as our own backbuffer, see
      // https://stackoverflow.com/questions/63759688/sdl-renderpresent-implementation
      // The SDL wiki also shows this technique of drawing to an in-memory texture buffer
      // before rendering to the backbuffer: https://wiki.libsdl.org/SDL3/SDL_CreateTexture
      sdlTexture = SDL_CreateTexture(sdlRenderer.get(), SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET, w, h);

      if (sdlTexture == NULL) {
        wm_log.error("could not create window texture: %s\n", SDL_GetError());
      }

      // Default to rendering all draw calls to the intermediate texture buffer
      SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);
      SDL_RenderClear(sdlRenderer.get());
    }

    size_t get_num_dialog_items(void) {
      return dialogItems.size();
    }

    std::shared_ptr<DialogItem> get_dialog_item(size_t index) {
      return dialogItems.at(index);
    }

    void sync(void) {
      SDL_SetRenderTarget(sdlRenderer.get(), NULL);
      SDL_RenderTexture(sdlRenderer.get(), sdlTexture, NULL, NULL);
      SDL_RenderPresent(sdlRenderer.get());
      SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);
      SDL_SyncWindow(sdlWindow);
    }

    bool draw_rect(const Rect& dispRect) {
      SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);

      if (!SDL_SetRenderDrawColor(sdlRenderer.get(), 255, 0, 0, 63)) {
        return false;
      }
      SDL_FRect bounds;
      bounds.x = dispRect.left;
      bounds.y = dispRect.top;
      bounds.w = dispRect.right - dispRect.left;
      bounds.h = dispRect.bottom - dispRect.top;
      if (!SDL_RenderRect(sdlRenderer.get(), &bounds)) {
        // Restore window as the render target
        SDL_SetRenderTarget(sdlRenderer.get(), NULL);
        return false;
      }

      // Restore window as the render target
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

      SDL_SetRenderDrawColor(sdlRenderer.get(), color.red, color.green, color.blue, 255);
      SDL_RenderLine(
          sdlRenderer.get(),
          static_cast<float>(start.h),
          static_cast<float>(start.v),
          static_cast<float>(end.h),
          static_cast<float>(end.v));

      // Restore window as the render target
      SDL_SetRenderTarget(sdlRenderer.get(), NULL);
    }

    void draw_background(PixPatHandle bkPixPat) {
      SDL_SetRenderTarget(sdlRenderer.get(), sdlTexture);

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

      // Restore window as the render target
      SDL_SetRenderTarget(sdlRenderer.get(), NULL);
    }

    void render() {
      SDL_RenderTexture(sdlRenderer.get(), sdlTexture, NULL, NULL);
      for (auto item : dialogItems) {
        item->render();
      }
    }

    void move(int hGlobal, int vGlobal) {
      SDL_SetWindowPosition(sdlWindow, hGlobal, vGlobal);
      SDL_SyncWindow(sdlWindow);
    }

    SDL_WindowID sdl_window_id() const {
      return SDL_GetWindowID(this->sdlWindow);
    }

  private:
    std::string title;
    Rect bounds;
    int w;
    int h;
    SDL_WindowFlags flags;
    SDL_Window* sdlWindow;
    std::shared_ptr<SDL_Renderer> sdlRenderer;
    std::vector<std::shared_ptr<DialogItem>> dialogItems;
    SDL_Texture* sdlTexture; // Use a texture to hold the window's rendered base canvas
  };

  WindowManager() = default;
  ~WindowManager() = default;

  WindowPtr create_window(
      const std::string& title,
      const Rect& bounds,
      bool visible,
      bool go_away,
      int16_t proc_id,
      uint32_t ref_con,
      const std::vector<ResourceDASM::ResourceFile::DecodedDialogItem>& dialog_items,
      SDL_WindowFlags flags) {
    CGrafPort port{};
    port.portRect = bounds;
    CWindowRecord* wr = new CWindowRecord();
    wr->port = port;
    wr->visible = visible;
    wr->goAwayFlag = go_away;
    wr->windowKind = proc_id;
    wr->refCon = ref_con;

    std::shared_ptr<Window> window = std::make_shared<Window>(title, bounds, flags, dialog_items);
    window->init();
    record_to_window.emplace(&wr->port, window);
    sdl_window_id_to_window.emplace(window->sdl_window_id(), window);

    return &wr->port;
  }

  void destroy_window(WindowPtr record) {
    auto window_it = record_to_window.find(record);
    if (window_it == record_to_window.end()) {
      throw std::logic_error("Attempted to delete nonexistent window");
    }
    sdl_window_id_to_window.erase(window_it->second->sdl_window_id());
    record_to_window.erase(window_it);
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

static void
load_fonts(void) {
  auto font_filename = host_filename_for_mac_filename(":Black Chancery.ttf", true);
  fonts_by_id[1601] = TTF_OpenFont(font_filename.c_str(), 16);
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

// See Macintosh Toolbox Essentials, 6-151
std::vector<ResourceDASM::ResourceFile::DecodedDialogItem> WindowManager_get_ditl_resources(int16_t ditlID) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_DITL, ditlID);
  return ResourceDASM::ResourceFile::decode_DITL(*data_handle, GetHandleSize(data_handle));
}

void WindowManager_Init(void) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    wm_log.error("Couldn't initialize video driver: %s\n", SDL_GetError());
    return;
  }

  PrintDebugInfo();

  TTF_Init();
  load_fonts();
}

WindowPtr WindowManager_CreateNewWindow(int16_t res_id, bool is_dialog, WindowPtr behind) {
  Rect bounds;
  int16_t proc_id;
  std::string title;
  bool visible;
  bool go_away;
  uint32_t ref_con;
  std::vector<ResourceDASM::ResourceFile::DecodedDialogItem> dialog_items;

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
    dialog_items = WindowManager_get_ditl_resources(dlog.items_id);

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
    dialog_items = std::vector<ResourceDASM::ResourceFile::DecodedDialogItem>();
  }

  SDL_WindowFlags flags{};

  if (proc_id == plainDBox) {
    flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY;
  }

  return wm.create_window(
      title,
      bounds,
      visible,
      go_away,
      proc_id,
      ref_con,
      dialog_items,
      flags);
}

void WindowManager_DrawDialog(WindowPtr theWindow) {
  CWindowRecord* const windowRecord = reinterpret_cast<CWindowRecord*>(theWindow);
  auto window = wm.window_for_record(theWindow);

  CGrafPtr port;
  GetPort(&port);
  if (port->bkPixPat) {
    window->draw_background(port->bkPixPat);
  }

  window->sync();
}

void WindowManager_MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, bool front) {
  if (theWindow == nullptr) {
    return;
  }

  auto window = wm.window_for_record(theWindow);
  window->move(hGlobal, vGlobal);
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
  window->sync();

  return noErr;
}

void GetDialogItem(DialogPtr theDialog, short itemNo, short* itemType, Handle* item, Rect* box) {
  auto window = wm.window_for_record(theDialog);
  if (itemNo > window->get_num_dialog_items()) {
    wm_log.error("Called GetDialogItem for itemNo %d on dialog %p that only has %d items", itemNo, theDialog, (int)window->get_num_dialog_items());
    return;
  }

  auto foundItem = window->get_dialog_item(itemNo - 1)->dialogItemHandle;

  *item = reinterpret_cast<Handle>(foundItem);
  *itemType = resource_type_to_int((*foundItem)->type);
  copy_rect(*box, (*foundItem)->bounds);
}

void GetDialogItemText(Handle item, Str255 text) {
  auto i = reinterpret_cast<DialogItemHandle>(item);
  memcpy(text, (*i)->text.c_str(), (*i)->text[0]);
}

int16_t StringWidth(ConstStr255Param s) {
  return s[0];
}

Boolean IsDialogEvent(const EventRecord* theEvent) {
  return false; // TODO
}

Boolean DialogSelect(const EventRecord* theEvent, DialogPtr* theDialog, short* itemHit) {
  return true; // TODO
}

void SystemClick(const EventRecord* theEvent, WindowPtr theWindow) {
  // TODO
}

void LineTo(int16_t h, int16_t v) {
  CGrafPtr port;
  GetPort(&port);
  auto window = wm.window_for_record(port);

  PixPatHandle ppat = port->pnPixPat;
  auto patData = (*ppat)->patData;
  auto pattern = *patData;

  RGBColor color{
      static_cast<uint16_t>(*pattern),
      static_cast<uint16_t>(*(pattern + 1)),
      static_cast<uint16_t>(*(pattern + 2))};

  Point end = Point{v, h};

  window->draw_line(port->pnLoc, end, color);
  port->pnLoc = end;

  window->sync();
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
    window->sync();
  } catch (std::out_of_range e) {
    wm_log.warning("Could not find window for current port");
    return;
  }
}

void SetDialogItemText(Handle item, ConstStr255Param text) {
  auto h = reinterpret_cast<DialogItemHandle>(item);
  (*h)->text = std::string(text[1], text[0] + 1);

  // TODO: Have to redraw the entire dialog right now, but there may be a better way
  //  of only selectively redrawing the modified text. Perhaps associating a separate
  //  SDL_Texture with each dialog item, then just re-compositing?
  //  Assume that the changed text appears on the current dialog's port
  CGrafPtr port;
  GetPort(&port);
  // WindowManager_DrawDialog(port);
}
