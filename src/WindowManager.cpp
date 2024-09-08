#include "WindowManager.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <phosg/Strings.hh>
#include <resource_file/ResourceFile.hh>

#include "MemoryManager.hpp"
#include "QuickDraw.hpp"

static phosg::PrefixedLogger wm_log("[WindowManager] ");

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
uint16_t WindowManager_get_ditl_resources(int16_t ditlID, DialogItem** items) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_DITL, ditlID);
  auto ditlData = read_from_handle(data_handle);

  uint16_t numItems = ditlData.get_u16b() + 1;
  *items = (DialogItem*)calloc(numItems, sizeof(DialogItem));
  for (int i = 0; i < numItems; i++) {
    ditlData.read(4); // reserved
    Rect dispWindow = rect_from_reader(ditlData);
    uint8_t type = ditlData.get_u8();
    bool enabled = (bool)(type & 0x80);
    type = (type & 0x7F);
    switch (type) {
      // PICT Resource
      case 64: {
        ditlData.read(1); // reserved
        uint16_t pictID = ditlData.get_u16b();
        auto p = GetPicture(pictID);
        (*items)[i].type = (*items)[i].DIALOG_ITEM_TYPE_PICT;
        (*items)[i].dialogItem.pict.dispRect = dispWindow;
        (*items)[i].dialogItem.pict.enabled = enabled;
        (*items)[i].dialogItem.pict.p = **p;
        break;
      }
      // Static Text
      case 8: {
        uint8_t length = ditlData.get_u8();
        (*items)[i].type = (*items)[i].DIALOG_ITEM_TYPE_STATIC_TEXT;
        (*items)[i].dialogItem.staticText.dispRect = dispWindow;
        (*items)[i].dialogItem.staticText.enabled = enabled;
        (*items)[i].dialogItem.staticText.text[0] = length;
        memcpy((*items)[i].dialogItem.staticText.text + 1, ditlData.read(length).c_str(), length);
        break;
      }
      default:
        wm_log.error("Unknown DITL type %d", type);
        break;
    }
  }
  return numItems;
}

void WindowManager_Init(void) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    wm_log.error("Couldn't initialize video driver: %s\n", SDL_GetError());
    return;
  }

  PrintDebugInfo();
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

  SDL_Window* window = SDL_CreateWindow(
      title.c_str(),
      bounds.right - bounds.left,
      bounds.bottom - bounds.top,
      flags);

  if (window == NULL) {
    wm_log.error("Could not create window: %s\n", SDL_GetError());
    return NULL;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, "opengl");

  if (renderer == NULL) {
    wm_log.error("could not create renderer: %s\n", SDL_GetError());
    return NULL;
  }

  CGrafPort port;
  port.portRect = bounds;
  CWindowRecord* wr = new CWindowRecord();
  wr->port = port;
  wr->visible = visible;
  wr->goAwayFlag = go_away;
  wr->windowKind = proc_id;
  wr->sdlWindow = window;
  wr->sdlRenderer = renderer;
  wr->refCon = ref_con;
  wr->numItems = num_dialog_items;
  wr->dItems = dialog_items;

  SDL_SyncWindow(wr->sdlWindow);
  return &wr->port;
}

void WindowManager_DrawDialog(WindowPtr theWindow) {
  CWindowRecord* const window = reinterpret_cast<CWindowRecord*>(theWindow);
  SDL_Renderer* renderer = window->sdlRenderer;

  SDL_RenderClear(renderer);

  for (int i = 0; i < window->numItems; i++) {
    DialogItem di = window->dItems[i];

    switch (di.type) {
      case di.DIALOG_ITEM_TYPE_PICT: {
        DialogItemPict pict = di.dialogItem.pict;
        Rect r = pict.p.picFrame;
        uint32_t w = r.right - r.left;
        uint32_t h = r.bottom - r.top;
        SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGB24, *pict.p.data, 3 * w);

        if (s == NULL) {
          wm_log.error("Could not create surface: %s\n", SDL_GetError());
          break;
        }

        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        if (t == NULL) {
          wm_log.error("Could not create texture: %s\n", SDL_GetError());
          break;
        }
        SDL_DestroySurface(s);

        SDL_FRect dest;
        dest.x = pict.dispRect.left;
        dest.y = pict.dispRect.top;
        dest.w = pict.dispRect.right - pict.dispRect.left;
        dest.h = pict.dispRect.bottom - pict.dispRect.top;

        SDL_RenderTexture(renderer, t, NULL, &dest);
        break;
      }
      case di.DIALOG_ITEM_TYPE_STATIC_TEXT: {
        // TODO: Render static text in dialogs
        break;
      }
    }
  }

  SDL_RenderPresent(renderer);
  SDL_SyncWindow(window->sdlWindow);

  SDL_Event e;
  bool quit = false;

  while (!quit) {
    SDL_bool success = SDL_WaitEventTimeout(&e, 100);

    if (success && e.type == SDL_EVENT_QUIT) {
      quit = true;
    }
  }
}

bool WindowManager_WaitNextEvent(EventRecord* theEvent) {
  SDL_Event e;
  SDL_bool success = SDL_PollEvent(&e);

  if (success == SDL_FALSE) {
    theEvent->what = nullEvent;
    return false;
  }

  switch (e.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      theEvent->what = mouseDown;
      theEvent->when = 60 * e.common.timestamp / 1000000000;
      theEvent->where.h = e.button.x;
      theEvent->where.v = e.button.y;
      break;
    default:
      theEvent->what = nullEvent;
      return false;
      break;
  }

  return true;
}

void WindowManager_MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, bool front) {
  if (theWindow == nullptr) {
    return;
  }

  CWindowRecord* const window = reinterpret_cast<CWindowRecord*>(theWindow);

  SDL_SetWindowPosition(window->sdlWindow, hGlobal, vGlobal);
  SDL_SyncWindow(window->sdlWindow);
}

void WindowManager_DisposeWindow(WindowPtr theWindow) {
  if (theWindow == nullptr) {
    return;
  }

  CWindowRecord* const window = reinterpret_cast<CWindowRecord*>(theWindow);

  SDL_DestroyRenderer(window->sdlRenderer);
  SDL_DestroyWindow(window->sdlWindow);

  free(window->dItems);
  delete window;
}

DisplayProperties WindowManager_GetPrimaryDisplayProperties(void) {
  auto displayID = SDL_GetPrimaryDisplay();

  if (displayID == 0) {
    wm_log.error("Could not get primary display: %s", SDL_GetError());
    return {};
  }

  SDL_Rect bounds{};
  if (SDL_GetDisplayBounds(displayID, &bounds) < 0) {
    wm_log.error("Could not get display bounds: %s", SDL_GetError());
    return {};
  }

  return DisplayProperties{
      bounds.w,
      bounds.h};
}
