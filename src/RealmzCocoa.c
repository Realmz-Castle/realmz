//
//  RealmzCocoa.c
//  Realmz
//
//  Created by Dan Applegate on 4/21/24.
//
#include "RealmzCocoa.h"

#include <SDL3/SDL_events.h>
#include <time.h>

#include "ResourceManager.h"
#include "SoundManager.h"
#include "WindowManager.h"

QuickDrawGlobals qd;
GDevice gDevice;
GDPtr gDevicePtr;
PrefRecord prefs;
PrefPtr prefsPtr;
PixMap gdPixMap;
PixMapPtr gdPixMapPtr;

static inline void cpyRect(const Rect* src, Rect* dst) {
  dst->top = src->top;
  dst->left = src->left;
  dst->bottom = src->bottom;
  dst->right = src->right;
}

// It appears that the GetPortBounds and ErasePortRect functions, which are not
// part of the Classic Mac system APIs, are only used to erase the entire screen.
// Since we'll be replacing this sort of imperative style with more modern, declarative
// windowing systems, it should be safe to make these no-ops.
void GetPortBounds(CGrafPtr port, Rect* rect) {
  // no-op
}

void ErasePortRect(void) {
  // no-op
}

void GetControlBounds(ControlHandle ctrl, Rect* rect) {
  *rect = (**ctrl).contrlRect;
}

void GetSoundVol(short* x) {
  uint32_t v;
  GetDefaultOutputVolume(&v);
  *x = v >> 16;
}

void BackColor(uint32_t color) {
}

void BackPixPat(PixPatHandle ppat) {
}

void BitClr(void* bytePtr, uint32_t bitNum) {
}

void BitSet(void* bytePtr, int32_t bitNum) {
}

Boolean BitTst(const void* bytePtr, int32_t bitNum) {
  return FALSE;
}

void DisposeDialog(DialogPtr theDialog) {
  WindowManager_DisposeWindow(theDialog);
}

void DrawDialog(DialogPtr theDialog) {
  WindowManager_DrawDialog(theDialog);
}

void DrawString(ConstStr255Param s) {
}

void EraseRect(const Rect* r) {
}

Boolean IsColorGammaAvailable(void) {
  return TRUE;
}

OSErr FadeToBlack(UInt16 numSteps, FadeType typeOfFade) {
  return 0;
}

OSErr FadeToGamma(GammaRef to, UInt16 numSteps, FadeType typeOfFade) {
  return 0;
}

void FlushEvents(uint16_t whichMask, uint16_t stopMask) {
  SDL_Event e;
  while (SDL_PollEvent(&e))
    ;
}

void ForeColor(uint32_t color) {
}

WindowPtr FrontWindow(void) {
  return NULL;
}

void DrawPicture(PicHandle myPicture, const Rect* dstRect) {
}

void GetBackColor(RGBColor* color) {
}

void GetForeColor(RGBColor* color) {
}

CCrsrHandle GetCCursor(uint16_t crsrID) {
  return NULL;
}

CIconHandle GetCIcon(uint16_t iconID) {
  return NULL;
}

OSErr GetDefaultOutputVolume(uint32_t* level) {
  return 0;
}

GDHandle GetGDevice(void) {
  return NULL;
}

void GetGWorld(CGrafPtr* port, GDHandle* gdh) {
}

PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld) {
  return NULL;
}

GDHandle GetMainDevice(void) {
  DisplayProperties dp = WindowManager_GetPrimaryDisplayProperties();
  gDevice.gdRect.top = 0;
  gDevice.gdRect.left = 0;
  gDevice.gdRect.right = dp.width;
  gDevice.gdRect.bottom = dp.height;

  gdPixMap.bounds = gDevice.gdRect;
  gdPixMap.pixelSize = 32;
  gdPixMapPtr = &gdPixMap;
  gDevice.gdPMap = &gdPixMapPtr;

  gDevicePtr = &gDevice;

  return &gDevicePtr;
}

DialogPtr GetNewDialog(uint16_t res_id, void* dStorage, WindowPtr behind) {
  return WindowManager_CreateNewWindow(res_id, true, behind);
}

CWindowPtr GetNewCWindow(int16_t res_id, void* wStorage, WindowPtr behind) {
  return WindowManager_CreateNewWindow(res_id, false, behind);
}

Boolean GetNextEvent(uint16_t eventMask, EventRecord* theEvent) {
  return FALSE;
}

void GetPort(GrafPtr* port) {
}

void HLockHi(Handle h) {
}

void BlockMove(const void* srcPtr, void* destPtr, Size byteCount) {
}

void HLock(Handle h) {
}

void HUnlock(Handle h) {
}

void BeginUpdate(WindowPtr theWindow) {
}

void GetDialogItem(DialogPtr theDialog, short itemNo, short* itemType, Handle* item, Rect* box) {
}

void TextFace(int16_t face) {
}

Boolean IsDialogEvent(const EventRecord* theEvent) {
  return FALSE;
}

Boolean DialogSelect(const EventRecord* theEvent, DialogPtr* theDialog, short* itemHit) {
  return TRUE;
}

int16_t HiWord(int32_t x) {
  return 0;
}

int16_t LoWord(int32_t x) {
  return 0;
}

void SetPt(Point* pt, int16_t h, int16_t v) {
}

int16_t DIBadMount(Point where, int32_t evtMessage) {
  return 0;
}

int32_t MenuKey(int16_t ch) {
  return 0;
}

int16_t FindWindow(Point thePoint, WindowPtr* theWindow) {
  return 0;
}

void HiliteMenu(int16_t menuID) {
}

int16_t StringWidth(ConstStr255Param s) {
  return 0;
}

void SetItemIcon(MenuHandle theMenu, int16_t item, int16_t iconIndex) {
}

void GetMouse(Point* mouseLoc) {
}

void LineTo(int16_t h, int16_t v) {
}

void DisposeWindow(WindowPtr theWindow) {
  WindowManager_DisposeWindow(theWindow);
}

ControlHandle GetNewControl(int16_t controlID, WindowPtr owner) {
  return NULL;
}

void MoveControl(ControlHandle theControl, int16_t h, int16_t v) {
}

void BringToFront(WindowPtr theWindow) {
}

void PenMode(int16_t mode) {
}

void GetDialogItemText(Handle item, Str255 text) {
}

void ParamText(ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3) {
}

int16_t GetControlValue(ControlHandle theControl) {
  return 0;
}

int16_t GetControlMinimum(ControlHandle theControl) {
  return 0;
}

int16_t GetControlMaximum(ControlHandle theControl) {
  return 0;
}

void ScrollRect(const Rect* r, int16_t dh, int16_t dv, RgnHandle updateRgn) {
}

int16_t TrackControl(ControlHandle theControl, Point thePoint, ProcPtr actionProc) {
  return 0;
}

void SetControlValue(ControlHandle theControl, int16_t theValue) {
}

void CopyBits(const BitMap* srcBits, const BitMap* dstBits, const Rect* srcRect, const Rect* dstRect, int16_t mode,
    RgnHandle maskRgn) {
}

void PenPixPat(PixPatHandle ppat) {
}

void SelectDialogItemText(DialogPtr theDialog, int16_t itemNo, int16_t strtSel, int16_t endSel) {
}

void SetDialogItemText(Handle item, ConstStr255Param text) {
}

RgnHandle NewRgn(void) {
  return NULL;
}

void RectRgn(RgnHandle rgn, const Rect* r) {
}

int32_t DragGrayRgn(RgnHandle theRgn, Point startPt, const Rect* boundsRect, const Rect* slopRect,
    int16_t axis, Ptr actionProc) {
  return 0;
}

Boolean Button(void) {
  return FALSE;
}

Boolean PtInRect(Point pt, const Rect* r) {
  return (pt.v >= r->top) && (pt.h >= r->left) && (pt.v < r->bottom) && (pt.h < r->right);
}

void ExitToShell(void) {
}

void LocalToGlobal(Point* pt) {
}

void GlobalToLocal(Point* pt) {
}

int16_t FindControl(Point thePoint, WindowPtr theWindow, ControlHandle* theControl) {
  return 0;
}

void BlockMoveData(const void* srcPtr, void* dstPtr, Size byteCount) {
}

int16_t Random(void) {
  return 0;
}

int16_t OpenDeskAcc(ConstStr255Param deskAccName) {
  return 0;
}

void SetItemMark(MenuHandle theMenu, int16_t item, int16_t markChar) {
}

void DisposeCCursor(CCrsrHandle cCrsr) {
}

void SelectWindow(WindowPtr theWindow) {
}

void EndUpdate(WindowPtr theWindow) {
}

Boolean LockPixels(PixMapHandle pm) {
  return FALSE;
}

Size MaxMem(Size* grow) {
  return 0;
}

int16_t CountMItems(MenuHandle theMenu) {
  return 0;
}

void ModalDialog(ModalFilterProcPtr filterProc, short* itemHit) {
}

void MoveTo(int16_t h, int16_t v) {
}

void MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, Boolean front) {
  WindowManager_MoveWindow(theWindow, hGlobal, vGlobal, front);
}

void CopyMask(const BitMap* srcBits, const BitMap* maskBits, const BitMap* dstBits, const Rect* srcRect, const Rect* maskRect,
    const Rect* dstRect) {
}

QDErr NewGWorld(GWorldPtr* offscreenGWorld, int16_t pixelDepth, const Rect* boundsRect, CTabHandle cTable,
    GDHandle aGDevice, GWorldFlags flags) {
  *offscreenGWorld = malloc(sizeof(CGrafPort));
  (*offscreenGWorld)->portRect.top = boundsRect->top;
  (*offscreenGWorld)->portRect.left = boundsRect->left;
  (*offscreenGWorld)->portRect.bottom = boundsRect->bottom;
  (*offscreenGWorld)->portRect.right = boundsRect->right;

  return 0;
}

void NumToString(int32_t theNum, Str255 theString) {
}

void OffsetRect(Rect* r, uint16_t dh, uint16_t dv) {
  r->left += dh;
  r->right += dh;
  r->top += dv;
  r->bottom += dv;
}

void PenSize(int16_t width, int16_t height) {
}

void RGBBackColor(const RGBColor* color) {
}

void RGBForeColor(const RGBColor* color) {
}

void SetCCursor(CCrsrHandle cCrsr) {
}

OSErr SetDefaultOutputVolume(uint32_t level) {
  return 0;
}

OSErr SetDepth(GDHandle aDevice, uint16_t depth, uint16_t whichFlags, uint16_t flags) {
  return 0;
}

void SetGWorld(CGrafPtr port, GDHandle gdh) {
}

void SetPort(GrafPtr port) {
}

void SetRect(Rect* r, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
  r->left = left;
  r->top = top,
  r->right = right;
  r->bottom = bottom;
}

void ShowWindow(WindowPtr theWindow) {
}

void SizeWindow(CWindowPtr theWindow, uint16_t w, uint16_t h, Boolean fUpdate) {
  theWindow->portRect.right = theWindow->portRect.left + w;
  theWindow->portRect.bottom = theWindow->portRect.top + h;
}

OSErr StartFading(GammaRef* returnedInitialState) {
  return 0;
}

void StopFading(GammaRef initialState, Boolean restore) {
}

void StringToNum(ConstStr255Param theString, int32_t* theNum) {
}

void SysBeep(uint16_t duration) {
}

void SystemTask(void) {
}

void TextFont(uint16_t font) {
}

void TextMode(int16_t mode) {
}

void TextSize(uint16_t size) {
}

uint32_t TickCount(void) {
  return clock() / CLOCKS_PER_SEC * 60.0;
}

void UnlockPixels(PixMapHandle pm) {
}

void PaintRect(const Rect* r) {
}

Boolean StillDown(void) {
  return FALSE;
}

void SystemClick(const EventRecord* theEvent, WindowPtr theWindow) {
}

void ObscureCursor(void) {
}

Boolean WaitNextEvent(int16_t eventMask, EventRecord* theEvent, uint32_t sleep, RgnHandle mouseRgn) {
  return WindowManager_WaitNextEvent(theEvent);
}

OSErr PlotCIcon(const Rect* theRect, CIconHandle theIcon) {
  return 0;
}

OSErr DisposeCIcon(CIconHandle theIcon) {
  return 0;
}

void InsetRect(Rect* r, int16_t dh, int16_t dv) {
}

Boolean SectRect(const Rect* src1, const Rect* src2, Rect* dstRect) {
  return FALSE;
}

void FrameOval(const Rect* r) {
}

void HideControl(ControlHandle theControl) {
}

void ShowControl(ControlHandle theControl) {
}

void SetControlMaximum(ControlHandle theControl, int16_t maxValue) {
}

void SizeControl(ControlHandle theControl, int16_t w, int16_t h) {
}

int32_t DeltaPoint(Point ptA, Point ptB) {
  return 0;
}

void FrameRect(const Rect* r) {
}

int16_t TextWidth(const void* textBuf, int16_t firstByte, int16_t byteCount) {
  return 0;
}

TEHandle TENew(const Rect* destRect, const Rect* viewRect) {
  return NULL;
}

void TESetText(const void* text, int32_t length, TEHandle hTE) {
}

void TESetSelect(int32_t selStart, int32_t selEnd, TEHandle hTE) {
}

void TEUpdate(const Rect* rUpdate, TEHandle hTE) {
}

void TEDispose(TEHandle hTE) {
}

void GetItemMark(MenuHandle theMenu, int16_t item, int16_t* markChar) {
}

void LMSetMBarHeight(int16_t h) {
}

void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn) {
}

RgnHandle GetGrayRgn(void) {
  return NULL;
}

void UnionRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) {
}

void SectRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) {
}

void PaintOne(WindowPeek window, RgnHandle clobberedRgn) {
}

void SFGetFile(Point where, const Str255 prompt, Ptr fileFilter, int16_t numTypes, SFTypeList typeList,
    Ptr dlgHook, SFReply* reply) {
}

void DrawControls(WindowPtr theWindow) {
}

TEHandle TEStyleNew(const Rect* destRect, const Rect* viewRect) {
  return NULL;
}

void TEStyleInsert(const void* text, int32_t length, StScrpHandle hSt, TEHandle hTE) {
}

void TESetAlignment(int16_t just, TEHandle hTE) {
}

void TEScroll(int16_t dh, int16_t dv, TEHandle hTE) {
}

void DisposeGWorld(GWorldPtr offscreenWorld) {
}

void DisposePixPat(PixPatHandle ppat) {
}

void SFPutFile(Point where, const Str255 prompt, const Str255 origName, Ptr dlgHook, SFReply* reply) {
}

OSErr GetProcessInformation(const ProcessSerialNumber* PSN, ProcessInfoRecPtr info) {
  return 0;
}

ControlHandle NewControl(WindowPtr theWindow, const Rect* boundsRect, ConstStr255Param title, Boolean visible,
    int16_t value, int16_t min, int16_t max, int16_t procID, int32_t refCon) {
  return NULL;
}

void HideCursor(void) {
}

void ShowCursor(void) {
}

void TEDelete(TEHandle hTE) {
}

void InitGraf(void* globalPtr) {
}

void InitWindows(void) {
  WindowManager_Init();
}

void InitRealmzCocoa() {
  // On Classic Mac OS, the system does this automatically when the
  // application is loaded.
  FSSpec spec;
  FSMakeFSSpec(0, 0, "\p:realmz", &spec);
  if (FSpOpenResFile(&spec, fsRdPerm) < 0) {
    fprintf(stderr, "WARNING: Cannot open the Realmz application resource file\n");
  }
}
