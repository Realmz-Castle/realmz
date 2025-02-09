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

WindowPtr FrontWindow(void) {
  return NULL;
}

CCrsrHandle GetCCursor(uint16_t crsrID) {
  return NULL;
}

OSErr GetDefaultOutputVolume(uint32_t* level) {
  return 0;
}

GDHandle GetGDevice(void) {
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

void BeginUpdate(WindowPtr theWindow) {
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
  if (thePoint.v < 0 && thePoint.h < 0) {
    return inMenuBar;
  }
  return 0;
}

void HiliteMenu(int16_t menuID) {
}

void SetItemIcon(MenuHandle theMenu, int16_t item, int16_t iconIndex) {
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

void SelectDialogItemText(DialogPtr theDialog, int16_t itemNo, int16_t strtSel, int16_t endSel) {
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

void ExitToShell(void) {
  exit(EXIT_SUCCESS);
}

void LocalToGlobal(Point* pt) {
}

void GlobalToLocal(Point* pt) {
}

int16_t FindControl(Point thePoint, WindowPtr theWindow, ControlHandle* theControl) {
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

void CopyMask(const BitMap* srcBits, const BitMap* maskBits, const BitMap* dstBits, const Rect* srcRect, const Rect* maskRect,
    const Rect* dstRect) {
}

void OffsetRect(Rect* r, uint16_t dh, uint16_t dv) {
  r->left += dh;
  r->right += dh;
  r->top += dv;
  r->bottom += dv;
}

void PenSize(int16_t width, int16_t height) {
}

void SetCCursor(CCrsrHandle cCrsr) {
}

OSErr SetDefaultOutputVolume(uint32_t level) {
  return 0;
}

OSErr SetDepth(GDHandle aDevice, uint16_t depth, uint16_t whichFlags, uint16_t flags) {
  return 0;
}

void SetRect(Rect* r, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
  r->left = left;
  r->top = top,
  r->right = right;
  r->bottom = bottom;
}

OSErr StartFading(GammaRef* returnedInitialState) {
  return 0;
}

void StopFading(GammaRef initialState, Boolean restore) {
}

void SysBeep(uint16_t duration) {
}

void SystemTask(void) {
}

void UnlockPixels(PixMapHandle pm) {
}

void PaintRect(const Rect* r) {
}

void ObscureCursor(void) {
}

OSErr DisposeCIcon(CIconHandle theIcon) {
  return 0;
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
