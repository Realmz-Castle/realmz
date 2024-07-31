#include "WindowManager.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

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
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("Couldn't initialize video driver: %s\n", SDL_GetError());
    return;
  }

  PrintDebugInfo();
}

WindowPtr WindowManager_CreateNewWindow(Rect bounds, char* title, bool visible, int procID, WindowPtr behind,
    bool goAwayFlag, int32_t refCon, uint16_t numItems, ResourceManager_DialogItem* dItems) {

  SDL_WindowFlags flags{};

  if (procID == plainDBox) {
    flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY;
  }

  SDL_Window* window = SDL_CreateWindow(
      title,
      bounds.right - bounds.left,
      bounds.bottom - bounds.top,
      flags);

  if (window == NULL) {
    SDL_Log("Could not create window: %s\n", SDL_GetError());
    return NULL;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, "opengl");

  if (renderer == NULL) {
    SDL_Log("could not create renderer: %s\n", SDL_GetError());
    return NULL;
  }

  CGrafPort port;
  port.portRect = bounds;
  CWindowRecord* wr = new CWindowRecord();
  wr->port = port;
  wr->visible = visible;
  wr->goAwayFlag = goAwayFlag;
  wr->windowKind = procID;
  wr->sdlWindow = window;
  wr->sdlRenderer = renderer;
  wr->refCon = refCon;
  wr->numItems = numItems;
  wr->dItems = dItems;

  SDL_SyncWindow(wr->sdlWindow);
  return &wr->port;
}

void WindowManager_DrawDialog(WindowPtr theWindow) {
  CWindowRecord* const window = reinterpret_cast<CWindowRecord*>(theWindow);
  SDL_Renderer* renderer = window->sdlRenderer;

  SDL_RenderClear(renderer);

  for (int i = 0; i < window->numItems; i++) {
    ResourceManager_DialogItem di = window->dItems[i];

    switch (di.type) {
      case di.DIALOG_ITEM_TYPE_PICT:
        ResourceManager_DialogItemPict pict = di.dialogItem.pict;
        ResourceManager_Rect r = pict.p.picFrame;
        uint32_t w = r.right - r.left;
        uint32_t h = r.bottom - r.top;
        SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGB24, (void*)pict.p.data, 3 * w);

        if (s == NULL) {
          SDL_Log("Could not create surface: %s\n", SDL_GetError());
          break;
        }

        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        if (t == NULL) {
          SDL_Log("Could not create texture: %s\n", SDL_GetError());
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
