#ifndef WindowManager_h
#define WindowManager_h

#include <stdbool.h>

#include "EventManager.h"
#include "QuickDraw.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// TEHandle is an opaque type accessible only in C++. We use this opaque struct
// in C so the type of the handle will be unique, and incorrectly-typed objects
// can't be passed to the various API functions.
struct _TextEditOpaque {
  int _opaque_contents;
};
typedef struct _TextEditOpaque* TEHandle;

typedef Ptr ModalFilterProcPtr;

typedef struct {
  // We don't use style scrap records, so this is unimplemented. The format is
  // ResourceDASM::StyleResourceCommand, but that's defined in C++ and can't be
  // used directly here.
  int _opaque_contents;
} StScrpRec;
typedef StScrpRec *StScrpPtr, **StScrpHandle;

// Macintosh Toolbox Essentials (Introduction to Windows 4-11)
enum {
  documentProc = 0, // movable, sizable window, no zoom box
  dBoxProc = 1, // alert box or modal dialog box
  plainDBox = 2, // plain box
  altDBoxProc = 3, // plain box with shadow
  noGrowDocProc = 4, // movable window, no size box or zoom box
  movableDBoxProc = 5, // movable modal dialog box
  zoomDocProc = 8, // standard document window
  zoomNoGrow = 12, // zoomable, nonresizable window
  rDocProc = 16, // rounded-corner window
};

enum {
  kControlNoPart = 0,
  kControlLabelPart = 1,
  kControlMenuPart = 2,
  kControlTrianglePart = 4,
  kControlEditTextPart = 5,
  kControlPicturePart = 6,
  kControlIconPart = 7,
  kControlClockPart = 8,
  kControlButtonPart = 10,
  kControlCheckBoxPart = 11,
  kControlRadioButtonPart = 12,
  kControlUpButtonPart = 20,
  kControlDownButtonPart = 21,
  kControlPageUpPart = 22,
  kControlPageDownPart = 23,
  kControlListBoxPart = 24,
  kControlListBoxDoubleClickPart = 25,
  kControlImageWellPart = 26,
  kControlRadioGroupPart = 27,
  kControlIndicatorPart = 129,
  kControlDisabledPart = 254,
  kControlInactivePart = 255,
};

typedef struct {
  int width;
  int height;
} DisplayProperties;

typedef CGrafPtr CWindowPtr;
typedef CWindowPtr WindowPtr, DialogPtr, WindowRef;

typedef Handle ControlHandle, DialogItemHandle;

void WindowManager_Init(void);
WindowPtr WindowManager_CreateNewWindow(int16_t res_id, bool is_dialog, WindowPtr behind);
void WindowManager_DrawDialog(WindowPtr theWindow);
void WindowManager_DisposeWindow(WindowPtr theWindow);
void GetDialogItem(DialogPtr theDialog, int16_t itemNo, int16_t* itemType, Handle* item, Rect* box);
void GetDialogItemText(Handle item, Str255 text);
void SetDialogItemText(Handle item, ConstStr255Param text);
int16_t StringWidth(ConstStr255Param s);
void SetDialogItemText(Handle item, ConstStr255Param text);
DialogPtr GetNewDialog(uint16_t dialogID, void* dStorage, WindowPtr behind);
WindowPtr GetNewCWindow(int16_t windowID, void* wStorage, WindowPtr behind);
void SizeWindow(WindowPtr theWindow, uint16_t w, uint16_t h, Boolean fUpdate);
void MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, Boolean front);
void ShowWindow(WindowPtr theWindow);

Boolean IsDialogEvent(const EventRecord* ev);
Boolean DialogSelect(const EventRecord* ev, DialogPtr* dlg, short* item_hit);
void SystemClick(const EventRecord* ev, WindowPtr window);
void DisposeWindow(WindowPtr theWindow);
WindowPtr FrontWindow();
int16_t FindWindow(Point p, WindowPtr* w);
void BringToFront(WindowPtr w);
void SelectWindow(WindowPtr w);
void NumToString(int32_t num, Str255 str);
void StringToNum(ConstStr255Param str, int32_t* num);
void ModalDialog(ModalFilterProcPtr filterProc, short* itemHit);
void DrawDialog(DialogPtr theDialog);
void DisposeDialog(DialogPtr theDialog);

ControlHandle GetNewControl(int16_t ctl_id, WindowPtr window);
ControlHandle NewControl(
    WindowPtr window,
    const Rect* bounds,
    ConstStr255Param title,
    Boolean visible,
    short value,
    short min,
    short max,
    short proc_id,
    long ref_con);
void ShowControl(ControlHandle ctl);
void HideControl(ControlHandle ctl);
void GetControlBounds(ControlHandle ctrl, Rect* rect);
void MoveControl(ControlHandle ctl, short h, short v);
void SizeControl(ControlHandle ctl, short w, short h);
void DrawControls(WindowPtr window);
short FindControl(Point pt, WindowPtr window, ControlHandle* ctl);
short TrackControl(ControlHandle ctl, Point pt, ProcPtr action_proc);
short GetControlValue(ControlHandle ctl);
short GetControlMinimum(ControlHandle ctl);
short GetControlMaximum(ControlHandle ctl);
void SetControlValue(ControlHandle ctl, short value);
void SetControlMinimum(ControlHandle ctl, short min);
void SetControlMaximum(ControlHandle ctl, short max);
void GetControlTitle(ControlHandle ctl, Str255 title);
TEHandle TENew(const Rect* destRect, const Rect* viewRect);
TEHandle TEStyleNew(const Rect* destRect, const Rect* viewRect);
void TEDispose(TEHandle hTE);
void TESetText(const void* text, int32_t length, TEHandle hTE);
void TEUpdate(const Rect* rUpdate, TEHandle hTE);
void TEStyleInsert(const void* text, int32_t length, StScrpHandle hSt, TEHandle hTE);
void TESetAlignment(int16_t just, TEHandle hTE);
void TEScroll(int16_t dh, int16_t dv, TEHandle hTE);

// Extensions for our implementation (not part of the original API)

// Our window manager implementation recomposites the entire window after each
// drawing call, which greatly amplifies the work required to draw parts of the
// UI, especially if e.g. a map viewed by the player covers a large area. To
// mitigate this, we disable recompositing temporarily in various places and
// manually recomposite after multiple drawing calls. None of the callsites of
// this function are part of the original source.
int WindowManager_SetEnableRecomposite(int enable);
void WindowManager_RecompositeAlways();

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // WindowManager_h
