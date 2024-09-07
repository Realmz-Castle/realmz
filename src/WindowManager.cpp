#include "WindowManager.h"

#include "FileManager.hpp"
#include "StringConvert.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <phosg/Strings.hh>
#include <resource_file/ResourceFile.hh>

#include "MemoryManager.hpp"
#include "QuickDraw.hpp"

using ResourceDASM::ResourceFile;

static phosg::PrefixedLogger wm_log("[WindowManager] ");
static std::unordered_map<int16_t, TTF_Font*> fonts_by_id;

class WindowManager {
public:
  class Window {
  public:
    Window() = default;
    Window(std::string title, const Rect& bounds, SDL_WindowFlags flags)
        : title{title},
          bounds{bounds},
          flags{flags} {}

    ~Window() {
      SDL_DestroyTexture(this->sdlTexture);
      SDL_DestroyRenderer(this->sdlRenderer);
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

      sdlRenderer = SDL_CreateRenderer(sdlWindow, "opengl");

      if (sdlRenderer == NULL) {
        wm_log.error("could not create renderer: %s\n", SDL_GetError());
        return;
      }

      // We'll use this texture as our own backbuffer, see
      // https://stackoverflow.com/questions/63759688/sdl-renderpresent-implementation
      // The SDL wiki also shows this technique of drawing to an in-memory texture buffer
      // before rendering to the backbuffer: https://wiki.libsdl.org/SDL3/SDL_CreateTexture
      sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET, w, h);

      if (sdlTexture == NULL) {
        wm_log.error("could not create window texture: %s\n", SDL_GetError());
      }

      // Default to rendering all draw calls to the intermediate texture buffer
      SDL_SetRenderTarget(sdlRenderer, sdlTexture);
      SDL_RenderClear(sdlRenderer);
    }

    void sync(void) {
      SDL_SetRenderTarget(sdlRenderer, NULL);
      SDL_RenderTexture(sdlRenderer, sdlTexture, NULL, NULL);
      SDL_RenderPresent(sdlRenderer);
      SDL_SetRenderTarget(sdlRenderer, sdlTexture);
      SDL_SyncWindow(sdlWindow);
    }

    void draw_rgba_picture(void* pixels, int w, int h, const Rect& dispRect) {
      SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, 4 * w);

      if (s == NULL) {
        wm_log.error("Could not create surface: %s\n", SDL_GetError());
        return;
      }

      SDL_Texture* t = SDL_CreateTextureFromSurface(sdlRenderer, s);
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

      SDL_RenderTexture(sdlRenderer, t, NULL, &dest);
    }

    bool draw_text(const Str255& text, const Rect& dispRect) {
      // TTF_Font* font = fonts_by_id.at(port->txFont);
      TTF_Font* font = fonts_by_id.at(1601);
      RGBColor fore_color;
      GetForeColor(&fore_color);
      SDL_Surface* text_surface = TTF_RenderText_Blended_Wrapped(
          font,
          reinterpret_cast<const char*>(&text[1]),
          static_cast<size_t>(text[0]),
          SDL_Color{
              static_cast<uint8_t>(fore_color.red / 0x0101),
              static_cast<uint8_t>(fore_color.green / 0x0101),
              static_cast<uint8_t>(fore_color.blue / 0x0101),
              255},
          dispRect.right - dispRect.left);
      if (!text_surface) {
        return false;
      }
      SDL_FRect text_dest;
      text_dest.x = dispRect.left;
      text_dest.y = dispRect.top;
      text_dest.w = std::min(dispRect.right - dispRect.left, text_surface->w);
      text_dest.h = std::min(dispRect.bottom - dispRect.top, text_surface->h);
      SDL_Texture* text_texture = SDL_CreateTextureFromSurface(sdlRenderer, text_surface);
      if (!text_texture || !SDL_RenderTexture(sdlRenderer, text_texture, NULL, &text_dest)) {
        return false;
      }
      return true;
    }

    void draw_background(PixPatHandle bkPixPat) {
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

      SDL_Texture* t = SDL_CreateTextureFromSurface(sdlRenderer, s);
      if (t == NULL) {
        wm_log.error("Could not create texture: %s\n", SDL_GetError());
        return;
      }
      SDL_DestroySurface(s);
      if (!SDL_RenderTextureTiled(sdlRenderer, t, NULL, 1.0, NULL)) {
        wm_log.error("Could not render background texture: %s", SDL_GetError());
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
    SDL_Renderer* sdlRenderer;
    SDL_Texture* sdlTexture; // Use a texture to hold the window's rendered state
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
      uint16_t num_dialog_items,
      DialogItem* dialog_items,
      SDL_WindowFlags flags) {
    CGrafPort port{};
    port.portRect = bounds;
    CWindowRecord* wr = new CWindowRecord();
    wr->port = port;
    wr->visible = visible;
    wr->goAwayFlag = go_away;
    wr->windowKind = proc_id;
    wr->refCon = ref_con;
    wr->numItems = num_dialog_items;
    wr->dItems = dialog_items;

    std::shared_ptr<Window> window = std::make_shared<Window>(title, bounds, flags);
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
    free(window->dItems);
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

void copy_rect(Rect& dst, ResourceDASM::Rect& src) {
  dst.top = src.y1;
  dst.left = src.x1;
  dst.bottom = src.y2;
  dst.right = src.x2;
}

// See Macintosh Toolbox Essentials, 6-151
uint16_t WindowManager_get_ditl_resources(int16_t ditlID, DialogItem** items) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_DITL, ditlID);
  auto ditl = ResourceDASM::ResourceFile::decode_DITL(*data_handle, GetHandleSize(data_handle));
  auto num_items = ditl.size();
  *items = reinterpret_cast<DialogItem*>(calloc(num_items, sizeof(DialogItem)));

  int i = 0;
  for (auto& ditl_item : ditl) {
    auto& item = (*items)[i++];
    copy_rect(item.dispRect, ditl_item.bounds);
    item.resource_id = ditl_item.resource_id;
    item.enabled = ditl_item.enabled;

    switch (ditl_item.type) {
      case ResourceFile::DecodedDialogItem::Type::BUTTON: // text valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_BUTTON;
        item.dialogItem.textual.text[0] = ditl_item.text.size();
        break;
      case ResourceFile::DecodedDialogItem::Type::CHECKBOX: // text valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_CHECKBOX;
        item.dialogItem.textual.text[0] = ditl_item.text.size();
        break;
      case ResourceFile::DecodedDialogItem::Type::RADIO_BUTTON: // text valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_RADIO_BUTTON;
        item.dialogItem.textual.text[0] = ditl_item.text.size();
        break;
      case ResourceFile::DecodedDialogItem::Type::TEXT: // text valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_TEXT;
        pstr_for_string<256>(item.dialogItem.textual.text, ditl_item.text);
        break;
      case ResourceFile::DecodedDialogItem::Type::EDIT_TEXT: // text valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_EDIT_TEXT;
        item.dialogItem.textual.text[0] = ditl_item.text.size();
        break;
      case ResourceFile::DecodedDialogItem::Type::UNKNOWN: // text contains raw info string (may be binary data!)
        item.type = DialogItem::TYPE::DIALOG_ITEM_UNKNOWN;
        item.dialogItem.textual.text[0] = ditl_item.text.size();
        break;
      case ResourceFile::DecodedDialogItem::Type::RESOURCE_CONTROL: // resource_id valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_RESOURCE_CONTROL;
        item.dialogItem.resource.res_id = ditl_item.resource_id;
        break;
      case ResourceFile::DecodedDialogItem::Type::ICON: // resource_id valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_ICON;
        item.dialogItem.resource.res_id = ditl_item.resource_id;
        break;
      case ResourceFile::DecodedDialogItem::Type::PICTURE: // resource_id valid
        item.type = DialogItem::TYPE::DIALOG_ITEM_PICTURE;
        item.dialogItem.resource.res_id = ditl_item.resource_id;
        break;
      case ResourceFile::DecodedDialogItem::Type::CUSTOM: // neither resource_id nor text valid
      default:
        break;
    }
  }

  return num_items;
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
  size_t num_dialog_items;
  DialogItem* dialog_items;

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
    num_dialog_items = WindowManager_get_ditl_resources(dlog.items_id, &dialog_items);

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
    num_dialog_items = 0;
    dialog_items = nullptr;
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
      num_dialog_items,
      dialog_items,
      flags);
}

void WindowManager_DrawDialog(WindowPtr theWindow) {
  CWindowRecord* const windowRecord = reinterpret_cast<CWindowRecord*>(theWindow);
  auto window = wm.window_for_record(theWindow);

  CGrafPtr port;
  GetPort(reinterpret_cast<GrafPtr*>(&port));
  if (port->bkPixPat) {
    window->draw_background(port->bkPixPat);
  }

  for (int i = 0; i < windowRecord->numItems; i++) {
    DialogItem di = windowRecord->dItems[i];

    switch (di.type) {
      case DialogItem::TYPE::DIALOG_ITEM_PICTURE: {
        auto pict = **GetPicture(di.dialogItem.resource.res_id);
        Rect r = pict.picFrame;
        uint32_t w = r.right - r.left;
        uint32_t h = r.bottom - r.top;
        window->draw_rgba_picture(*pict.data, w, h, di.dispRect);
        break;
      }
      case DialogItem::TYPE::DIALOG_ITEM_TEXT: {
        if (di.dialogItem.textual.text[0] < 1) {
          continue;
        }
        CGrafPtr port;
        GetPort(reinterpret_cast<GrafPtr*>(&port));
        if (!window->draw_text(di.dialogItem.textual.text, di.dispRect)) {
          wm_log.error("Error when rendering text item %d: %s", di.resource_id, SDL_GetError());
          continue;
        }
        break;
      }
      default:
        // TODO: Render other DITL types
        break;
    }
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
  auto windowRecord = reinterpret_cast<CWindowRecord*>(theDialog);
  if (itemNo > windowRecord->numItems) {
    wm_log.error("Called GetDialogItem for itemNo %d on dialog %p that only has %d items", itemNo, theDialog, windowRecord->numItems);
    return;
  }

  auto foundItem = windowRecord->dItems[itemNo - 1];

  // TODO: Figure out best way to return a handle to the foundItem in item
  *item = reinterpret_cast<Handle>(&foundItem);
  *itemType = foundItem.type;
  *box = foundItem.dispRect;
}

void GetDialogItemText(Handle item, Str255 text) {
  auto i = reinterpret_cast<DialogItem*>(item);
  memcpy(text, i->dialogItem.textual.text, i->dialogItem.textual.text[0]);
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
