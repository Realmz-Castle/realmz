#ifndef WindowManager_h
#define WindowManager_h

#include <stdbool.h>

#include "EventManager.h"
#include "QuickDraw.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef Ptr ModalFilterProcPtr;

// Macintosh Toolbox Essentials (Introduction to Windows 4-11)
enum {
  plainDBox = 2,
};

typedef struct {
  int width;
  int height;
} DisplayProperties;

typedef struct {
  CGrafPort port;
  int16_t windowKind;
  Boolean visible;
  Boolean goAwayFlag;
  StringHandle titleHandle;
  uint32_t refCon;
} CWindowRecord;
typedef CGrafPtr CWindowPtr;
typedef CWindowPtr WindowPtr, DialogPtr, WindowRef;

typedef struct {
  Rect contrlRect;
} ControlRecord;
typedef ControlRecord* ControlPtr;
typedef ControlPtr* ControlHandle;

void WindowManager_Init(void);
WindowPtr WindowManager_CreateNewWindow(int16_t res_id, bool is_dialog, WindowPtr behind);
void WindowManager_DrawDialog(WindowPtr theWindow);
void WindowManager_DisposeWindow(WindowPtr theWindow);
DisplayProperties WindowManager_GetPrimaryDisplayProperties(void);
void GetDialogItem(DialogPtr theDialog, int16_t itemNo, int16_t* itemType, Handle* item, Rect* box);
void GetDialogItemText(Handle item, Str255 text);
void SetDialogItemText(Handle item, ConstStr255Param text);
int16_t StringWidth(ConstStr255Param s);
void LineTo(int16_t h, int16_t v);
void DrawPicture(PicHandle myPicture, const Rect* dstRect);
void SetDialogItemText(Handle item, ConstStr255Param text);
DialogPtr GetNewDialog(uint16_t dialogID, void* dStorage, WindowPtr behind);
WindowPtr GetNewCWindow(int16_t windowID, void* wStorage, WindowPtr behind);
void SizeWindow(WindowPtr theWindow, uint16_t w, uint16_t h, Boolean fUpdate);
void MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, Boolean front);
void ShowWindow(WindowPtr theWindow);

Boolean IsDialogEvent(const EventRecord* ev);
Boolean DialogSelect(const EventRecord* ev, DialogPtr* dlg, short* item_hit);
void SystemClick(const EventRecord* ev, WindowPtr window);

Boolean IsDialogEvent(const EventRecord* ev);
Boolean DialogSelect(const EventRecord* ev, DialogPtr* dlg, short* item_hit);
void SystemClick(const EventRecord* ev, WindowPtr window);
void DisposeWindow(WindowPtr theWindow);
ControlHandle GetNewControl(int16_t controlID, WindowPtr owner);
void BringToFront(WindowPtr theWindow);
void ParamText(ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3);
void NumToString(int32_t num, Str255 str);
void StringToNum(ConstStr255Param str, int32_t* num);
void ModalDialog(ModalFilterProcPtr filterProc, short* itemHit);
void DrawDialog(DialogPtr theDialog);
void DisposeDialog(DialogPtr theDialog);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // WindowManager_h
