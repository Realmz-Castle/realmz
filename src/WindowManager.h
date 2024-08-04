#ifndef WindowManager_h
#define WindowManager_h

#include <SDL3/SDL.h>

#include "EventManager.h"
#include "QuickDraw.h"
#include "ResourceManager.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Macintosh Toolbox Essentials (Introduction to Windows 4-11)
enum {
  plainDBox = 2,
};

typedef struct {
  int width;
  int height;
} DisplayProperties;

typedef struct WindowResource {
  Rect portBounds;
  int16_t procID;
  bool visible;
  bool dismissable;
  uint32_t refCon;
  char windowTitle[256];
  uint16_t posSpec;
} WindowResource;

typedef struct DialogResource {
  Rect bounds;
  int16_t wDefID;
  bool visible;
  bool dismissable;
  uint32_t refCon;
  int16_t ditlID;
} DialogResource;

typedef struct DialogItemPict {
  Rect dispRect;
  bool enabled;
  Picture p;
} DialogItemPict;

typedef union DialogItemType {
  DialogItemPict pict;
} DialogItemType;

typedef struct DialogItem {
  enum DIALOG_ITEM_TYPE {
    DIALOG_ITEM_TYPE_PICT,
  } type;
  DialogItemType dialogItem;
} DialogItem;

typedef struct CWindowRecord {
  CGrafPort port;
  int16_t windowKind;
  Boolean visible;
  Boolean goAwayFlag;
  StringHandle titleHandle;
  uint32_t refCon;

  SDL_Window* sdlWindow;
  SDL_Renderer* sdlRenderer;
  uint16_t numItems;
  DialogItem* dItems;
} CWindowRecord;
typedef CGrafPtr CWindowPtr;
typedef CWindowPtr WindowPtr, DialogPtr, WindowRef;

WindowResource WindowManager_get_wind_resource(int16_t windowID);
DialogResource WindowManager_get_dlog_resource(int16_t dialogID);
uint16_t WindowManager_get_ditl_resources(int16_t ditlID, DialogItem** items);

void WindowManager_Init(void);
WindowPtr WindowManager_CreateNewWindow(Rect bounds, char* title, bool visible, int procID, WindowPtr behind,
    bool goAwayFlag, int32_t refCon, uint16_t numItems, DialogItem* dItems);
void WindowManager_DrawDialog(WindowPtr theWindow);
bool WindowManager_WaitNextEvent(EventRecord* theEvent);
void WindowManager_MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, bool front);
void WindowManager_DisposeWindow(WindowPtr theWindow);
DisplayProperties WindowManager_GetPrimaryDisplayProperties(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // WindowManager_h
