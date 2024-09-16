#include "WindowManager.h"

#include "FileManager.hpp"
#include "StringConvert.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
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

static void load_fonts(void) {
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
      case DialogItem::TYPE::DIALOG_ITEM_PICTURE: {
        auto pict = **GetPicture(di.dialogItem.resource.res_id);
        Rect r = pict.picFrame;
        uint32_t w = r.right - r.left;
        uint32_t h = r.bottom - r.top;
        SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGB24, *pict.data, 3 * w);

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
        dest.x = di.dispRect.left;
        dest.y = di.dispRect.top;
        dest.w = di.dispRect.right - di.dispRect.left;
        dest.h = di.dispRect.bottom - di.dispRect.top;

        SDL_RenderTexture(renderer, t, NULL, &dest);
        break;
      }
      case DialogItem::TYPE::DIALOG_ITEM_TEXT: {
        if (di.dialogItem.textual.text[0] < 1) {
          continue;
        }
        CGrafPtr port;
        GetPort(reinterpret_cast<GrafPtr*>(&port));
        // TTF_Font* font = fonts_by_id.at(port->txFont);
        TTF_Font* font = fonts_by_id.at(1601);
        RGBColor fore_color;
        GetForeColor(&fore_color);
        SDL_Surface* text_surface = TTF_RenderUTF8_Blended_Wrapped(
            font,
            string_for_pstr<256>(di.dialogItem.textual.text).c_str(),
            SDL_Color{
                static_cast<uint8_t>(fore_color.red),
                static_cast<uint8_t>(fore_color.green),
                static_cast<uint8_t>(fore_color.blue),
                255},
            di.dispRect.right - di.dispRect.left);
        if (!text_surface) {
          wm_log.error("Error when rendering text item %d: %s", di.resource_id, SDL_GetError());
          continue;
        }
        SDL_FRect text_dest;
        text_dest.x = di.dispRect.left;
        text_dest.y = di.dispRect.top;
        text_dest.w = std::min(di.dispRect.right - di.dispRect.left, text_surface->w);
        text_dest.h = std::min(di.dispRect.bottom - di.dispRect.top, text_surface->h);
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        if (!text_texture) {
          wm_log.error("Error when creating text texture for item %d: %s", di.resource_id, SDL_GetError());
          continue;
        }
        SDL_RenderTexture(renderer, text_texture, NULL, &text_dest);
        break;
      }
      default:
        // TODO: Render other DITL types
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
