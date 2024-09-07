#ifndef WindowManager_h
#define WindowManager_h

#include <SDL3/SDL.h>
#include <stdbool.h>

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

// Represents any item whose representation is solely text
typedef struct {
  Str255 text;
} DialogItemText;

typedef struct {
  int16_t res_id;
} DialogItemResourceId;

typedef union {
  DialogItemText textual;
  DialogItemResourceId resource;
} DialogItemParams;

typedef struct {
  int16_t resource_id;
  enum TYPE {
    DIALOG_ITEM_BUTTON,
    DIALOG_ITEM_CHECKBOX, // text valid
    DIALOG_ITEM_RADIO_BUTTON, // text valid
    DIALOG_ITEM_RESOURCE_CONTROL, // resource_id valid
    DIALOG_ITEM_TEXT, // text valid
    DIALOG_ITEM_EDIT_TEXT, // text valid
    DIALOG_ITEM_ICON, // resource_id valid
    DIALOG_ITEM_PICTURE, // resource_id valid
    DIALOG_ITEM_CUSTOM, // neither resource_id nor text valid
    DIALOG_ITEM_UNKNOWN, // text contains raw info string (may be binary data!)
  } type;
  DialogItemParams dialogItem;
  Rect dispRect;
  bool enabled;
} DialogItem;

typedef struct {
  CGrafPort port;
  int16_t windowKind;
  Boolean visible;
  Boolean goAwayFlag;
  StringHandle titleHandle;
  uint32_t refCon;

  uint16_t numItems;
  DialogItem* dItems;
} CWindowRecord;
typedef CGrafPtr CWindowPtr;
typedef CWindowPtr WindowPtr, DialogPtr, WindowRef;

uint16_t WindowManager_get_ditl_resources(int16_t ditlID, DialogItem** items);

void WindowManager_Init(void);
WindowPtr WindowManager_CreateNewWindow(int16_t res_id, bool is_dialog, WindowPtr behind);
void WindowManager_DrawDialog(WindowPtr theWindow);
void WindowManager_MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, bool front);
void WindowManager_DisposeWindow(WindowPtr theWindow);
DisplayProperties WindowManager_GetPrimaryDisplayProperties(void);
OSErr PlotCIcon(const Rect* theRect, CIconHandle theIcon);
void GetDialogItem(DialogPtr theDialog, int16_t itemNo, int16_t* itemType, Handle* item, Rect* box);
void GetDialogItemText(Handle item, Str255 text);
int16_t StringWidth(ConstStr255Param s);

Boolean IsDialogEvent(const EventRecord* ev);
Boolean DialogSelect(const EventRecord* ev, DialogPtr* dlg, short* item_hit);
void SystemClick(const EventRecord* ev, WindowPtr window);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // WindowManager_h
