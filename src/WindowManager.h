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

typedef struct {
  Rect portBounds;
  int16_t procID;
  bool visible;
  bool dismissable;
  uint32_t refCon;
  char windowTitle[256];
  uint16_t posSpec;
} WindowResource;

typedef struct {
  Rect bounds;
  int16_t wDefID;
  bool visible;
  bool dismissable;
  uint32_t refCon;
  int16_t ditlID;
} DialogResource;

typedef struct {
  Rect dispRect;
  bool enabled;
  Picture p;
} DialogItemPict;

typedef union {
  DialogItemPict pict;
} DialogItemType;

typedef struct {
  enum DIALOG_ITEM_TYPE {
    DIALOG_ITEM_TYPE_PICT,
  } type;
  DialogItemType dialogItem;
} DialogItem;

typedef struct {
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

uint16_t WindowManager_get_ditl_resources(int16_t ditlID, DialogItem** items);

void WindowManager_Init(void);
WindowPtr WindowManager_CreateNewWindow(int16_t res_id, bool is_dialog, WindowPtr behind);
void WindowManager_DrawDialog(WindowPtr theWindow);
bool WindowManager_WaitNextEvent(EventRecord* theEvent);
void WindowManager_MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, bool front);
void WindowManager_DisposeWindow(WindowPtr theWindow);
DisplayProperties WindowManager_GetPrimaryDisplayProperties(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // WindowManager_h
