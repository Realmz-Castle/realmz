//
//  RealmzCocoa.h
//  Realmz
//
//  Created by Dan Applegate on 3/1/24.
//

#ifndef RealmzCocoa_h
#define RealmzCocoa_h

#define pascal

#include <ctype.h>
#include <libkern/OSByteOrder.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FileManager.h"
#include "MemoryManager.h"
#include "MenuManager-C-Interface.h"
#include "QuickDraw.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Types.h"
#include "WindowManager.h"

#define SWAP_BIG16(x) OSSwapBigToHostInt16(x)
#define SWAP_BIG32(x) OSSwapBigToHostInt32(x)

#if !defined(TRUE)
#define TRUE true
#endif
#if !defined(FALSE)
#define FALSE false
#endif
#define SetPortDialogPort SetPort
#define GetDialogPort(x) ((CGrafPtr) & (((DialogPeek)(x))->window.port))
#define GetDialogFromWindow(x) (DialogPtr)(x)
#define GetQDGlobalsThePort() ((CGrafPtr)qd.thePort)
#define GetQDGlobalsScreenBits(x) (*(x) = qd.screenBits)
#define GetPortBitMapForCopyBits(x) ((BitMap*)&(((GrafPtr)(x))->portBits))
#define QDFlushPortBuffer(x, y)
#define GetPortPixMap(x) &(((CGrafPtr)(x))->portPixMap);
#define GetPortTextFont(x) ((x)->txFont)
#define GetPortTextFace(x) ((x)->txFace)
#define GetPortTextSize(x) ((x)->txSize)
#define GetPortTextMode(x) ((x)->txMode)

// Conversion functions from convert.h
// Should be no-ops with proper loading of resources
#define CvtTabItemToPc(x, y)
#define CvtTabItemAttrToPc(x, y)
#define CvtTabCharacterToPc(x, y)
#define CvtTabMonsterToPc(x, y)
#define CvtTabMapStatToPc(x, y)
#define CvtTabDoorToPc(x, y)
#define CvtTabShortToPc(x, y)
#define CvtCharacterToPc(x)
#define CvtThiefToPc(x)
#define CvtCasteToPc(x)
#define CvtDoorToPc(x)
#define CvtItemToPc(x)
#define CvtRaceToPc(x)
#define CvtMapsToPc(x)
#define CvtRandLevelToPc(x)
#define CvtMonsterToPc(x)
#define CvtNoteToPc(x)
#define CvtEncount2ToPc(x)
#define CvtEncountToPc(x)
#define CvtBattleToPc(x)
#define CvtTimeEncounterToPc(x)
#define CvtShopToPc(x)
#define CvtPrefsToPc(x)
#define CvtTreasureToPc(x)
#define CvtRestrictionInfoToPc(x)
#define CvtFieldToPc(x)
#define CvtShortToPc(x)
#define CvtLongToPc(x)
#define CvtFloatToPc(x)
#define CvtTabLongToPc(x, y)

// Spells are already composed solely of bytes.
#define CvtTabSpellToPc(x, y)

// contactdata is a bunch of static strings.
#define CvtContactToPc(x)

// These are just arrays of shorts.
#define CvtLayoutToPc(x) CvtTabShortToPc(x, 8 * 16)

#define GetNewWindow GetNewCWindow

static inline void rintel2moto(Rect* r) {
  r->top = SWAP_BIG16(r->top);
  r->left = SWAP_BIG16(r->left);
  r->bottom = SWAP_BIG16(r->bottom);
  r->right = SWAP_BIG16(r->right);
}

#define suspendResumeMessage 0x01

#define kControlUpButtonPart 20
#define kControlDownButtonPart 21
#define kControlPageUpPart 22
#define kControlPageDownPart 23
#define kControlIndicatorPart 129

#define inMenuBar 1
#define inSysWindow 2
#define inContent 3

#define resNotFound -192

#define srcCopy 0

#define smSystemScript -1

#define transparent 36

// "pref"
#define kPreferencesFolderType 0x70726566
#define kDontCreateFolder FALSE
// https://developer.apple.com/documentation/coreservices/1389331-anonymous/konsystemdisk
#define kOnSystemDisk -32768L

#define kCurrentProcess 2

#define userItem 0

#define centerMainScreen 0x280A

static inline void PtoCstr(Str255 x) {
  unsigned char len = x[0];

  for (int i = 0; i < len; i++) {
    x[i] = x[i + 1];
  }
  x[len] = '\0';
}

static inline void P2CStr(Str255 x) {
  PtoCstr(x);
}

static inline void CtoPstr(char* x) {
  unsigned char len = 0;
  unsigned char i = 0;
  while (x[i++] != '\0') {
  };
  len = i;
  while (--i > 0) {
    x[i] = x[i - 1];
  }
  x[0] = len - 1;
}

static inline void C2PStr(char* x) {
  CtoPstr(x);
}

typedef struct {
  Rect gdRect;
  PixMapHandle gdPMap;
} GDevice;

typedef struct {
  Rect contrlRect;
} ControlRecord;

typedef struct {
} ColorTable;

typedef struct {
  PixMap iconPMap;
  BitMap iconMask;
  BitMap iconBMap;
} CIcon;

typedef GDevice *GDPtr, **GDHandle;
typedef ControlRecord* ControlPtr;
typedef ControlPtr* ControlHandle;
typedef ColorTable* CTabPtr;
typedef CTabPtr* CTabHandle;
typedef CIcon *CIconPtr, **CIconHandle;
typedef Handle GammaTblHandle;
typedef Handle RgnHandle;
typedef Handle TEHandle;
typedef Handle CCrsrHandle;
typedef Handle SndListHandle;
typedef Ptr ModalFilterProcPtr;
typedef DialogPtr DialogRef;
typedef int16_t QDErr;
typedef BitMap* BitMapPtr;

typedef struct {
  CGrafPort port;
} WindowRecord;
typedef WindowRecord* WindowPeek;

typedef struct {
  WindowRecord window;
} DialogRecord;
typedef DialogRecord* DialogPeek;

typedef struct {
  GrafPtr thePort;
  BitMap screenBits;
} QuickDrawGlobals;

typedef struct {
  Boolean good;
  Str63 fName;
  int16_t vRefNum;
} SFReply;
typedef OSType SFTypeList[4];

typedef struct {

} StScrpRec;
typedef StScrpRec *StScrpPtr, **StScrpHandle;

typedef struct {
  uint32_t processInfoLength;
  StringPtr processName;
  FSSpecPtr processAppSpec;
} ProcessInfoRec;
typedef ProcessInfoRec* ProcessInfoRecPtr;

void LocalToGlobal(Point* pt);
PixPatHandle GetPixPat(uint16_t patID);
GDHandle GetMainDevice(void);
DialogPtr GetNewDialog(uint16_t dialogID, void* dStorage, WindowPtr behind);

void SysBeep(uint16_t duration);
void FlushEvents(uint16_t whichMask, uint16_t stopMask);
#define everyEvent -1
#define charCodeMask 0x000000FF
void ModalDialog(ModalFilterProcPtr filterProc, short* itemHit);
int16_t TrackControl(ControlHandle theControl, Point thePoint, ProcPtr actionProc);
void GetDialogItem(DialogPtr theDialog, int16_t itemNo, int16_t* itemType, Handle* item, Rect* box);
void SelectDialogItemText(DialogPtr theDialog, int16_t itemNo, int16_t strtSel, int16_t endSel);
void SetDialogItemText(Handle item, ConstStr255Param text);
void GetDialogItemText(Handle item, Str255 text);
Boolean Button(void);
RgnHandle NewRgn(void);
void RectRgn(RgnHandle rgn, const Rect* r);
Boolean PtInRect(Point pt, const Rect* r);
int32_t DragGrayRgn(RgnHandle theRgn, Point startPt, const Rect* boundsRect, const Rect* slopRect,
    int16_t axis, Ptr actionProc);
void ParamText(ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3);
void SystemTask(void);
Boolean GetNextEvent(uint16_t eventMask, EventRecord* theEvent);
void DisposeDialog(DialogPtr theDialog);
void ExitToShell(void);
OSErr SetDepth(GDHandle aDevice, uint16_t depth, uint16_t whichFlags, uint16_t flags);
OSErr GetDefaultOutputVolume(uint32_t* level);
void GetSoundVol(short* x);
OSErr SetDefaultOutputVolume(uint32_t level);
#define SetSoundVol SetDefaultOutputVolume
CCrsrHandle GetCCursor(uint16_t crsrID);
void SetCCursor(CCrsrHandle cCrsr);
void HLock(Handle h);
void HUnlock(Handle h);
WindowPtr GetNewCWindow(int16_t windowID, void* wStorage, WindowPtr behind);
void SizeWindow(WindowPtr theWindow, uint16_t w, uint16_t h, Boolean fUpdate);
void MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, Boolean front);
void ShowWindow(WindowPtr theWindow);
WindowPtr FrontWindow(void);
void TextFont(uint16_t font);
int32_t MenuKey(int16_t ch);
void HiliteMenu(int16_t menuID);
int16_t FindWindow(Point thePoint, WindowPtr* theWindow);
void DrawDialog(DialogPtr theDialog);
void TextSize(uint16_t size);
void DrawPicture(PicHandle myPicture, const Rect* dstRect);
void PenPixPat(PixPatHandle ppat);
GDHandle GetGDevice(void);
uint32_t TickCount(void);
Boolean IsDialogEvent(const EventRecord* theEvent);
Boolean DialogSelect(const EventRecord* theEvent, DialogPtr* theDialog, short* itemHit);
CIconHandle GetCIcon(uint16_t iconID);
void SetRect(Rect* r, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
Boolean LockPixels(PixMapHandle pm);
void UnlockPixels(PixMapHandle pm);
PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld);
Size MaxMem(Size* grow);
void BitClr(void* bytePtr, uint32_t bitNum);
void BackPixPat(PixPatHandle ppat);
Boolean WaitNextEvent(int16_t eventMask, EventRecord* theEvent, uint32_t sleep, RgnHandle mouseRgn);
void NumToString(int32_t theNum, Str255 theString);
void StringToNum(ConstStr255Param theString, int32_t* theNum);
void TextMode(int16_t mode);
void TextFace(int16_t face);
void DrawString(ConstStr255Param s);
int16_t StringWidth(ConstStr255Param s);
void GetMouse(Point* mouseLoc);
void SetItemIcon(MenuHandle theMenu, int16_t item, int16_t iconIndex);
void MoveTo(int16_t h, int16_t v);
Boolean BitTst(const void* bytePtr, int32_t bitNum);
void BitSet(void* bytePtr, int32_t bitNum);
void EraseRect(const Rect* r);
void OffsetRect(Rect* r, uint16_t dh, uint16_t dv);
void GetGWorld(CGrafPtr* port, GDHandle* gdh);
typedef uint32_t GWorldFlags;
QDErr NewGWorld(GWorldPtr* offscreenGWorld, int16_t pixelDepth, const Rect* boundsRect, CTabHandle cTable,
    GDHandle aGDevice, GWorldFlags flags);
void SetGWorld(CGrafPtr port, GDHandle gdh);
void PenSize(int16_t width, int16_t height);

typedef struct OpaqueGammaInfo** GammaRef;
// ----------
// Fade types
typedef UInt16 FadeType;
enum {
  inverseFade = (1 << 0), // invert fade flag (for nonlinear fades)

  linearFade = (1 << 1),
  quadraticFade = (1 << 2),
  inverseQuadraticFade = quadraticFade | inverseFade
};
Boolean IsColorGammaAvailable(void);
OSErr StartFading(GammaRef* returnedInitialState);
void StopFading(GammaRef initialState, Boolean restore);
OSErr FadeToBlack(UInt16 numSteps, FadeType typeOfFade);
OSErr FadeToGamma(GammaRef to, UInt16 numSteps, FadeType typeOfFade);
Handle Get1Resource(ResType theType, int16_t theID);
void HLockHi(Handle h);
void BlockMove(const void* srcPtr, void* destPtr, Size byteCount);
void BeginUpdate(WindowPtr theWindow);
void EndUpdate(WindowPtr theWindow);
void SetPt(Point* pt, int16_t h, int16_t v);
int16_t DIBadMount(Point where, int32_t evtMessage);
void LineTo(int16_t h, int16_t v);
void DisposeWindow(WindowPtr theWindow);
ControlHandle GetNewControl(int16_t controlID, WindowPtr owner);
void MoveControl(ControlHandle theControl, int16_t h, int16_t v);
void BringToFront(WindowPtr theWindow);
void PenMode(int16_t mode);
int16_t GetControlValue(ControlHandle theControl);
void SetControlValue(ControlHandle theControl, int16_t theValue);
int16_t GetControlMinimum(ControlHandle theControl);
int16_t GetControlMaximum(ControlHandle theControl);
void ScrollRect(const Rect* r, int16_t dh, int16_t dv, RgnHandle updateRgn);
void CopyBits(const BitMap* srcBits, const BitMap* dstBits, const Rect* srcRect, const Rect* dstRect, int16_t mode,
    RgnHandle maskRgn);
void GlobalToLocal(Point* pt);
void LocalToGlobal(Point* pt);
int16_t FindControl(Point thePoint, WindowPtr theWindow, ControlHandle* theControl);
void BlockMoveData(const void* srcPtr, void* dstPtr, Size byteCount);
int16_t Random(void);

#define GetDialogWindow(pDialog) (pDialog)
#define GetWindowPort(x) (x)
#define BitAnd(x, y) (x & y)
int16_t HiWord(int32_t x);
int16_t LoWord(int32_t x);
int16_t OpenDeskAcc(ConstStr255Param deskAccName);
void SetItemMark(MenuHandle theMenu, int16_t item, int16_t markChar);
void DisposeCCursor(CCrsrHandle cCrsr);
void SelectWindow(WindowPtr theWindow);
int16_t CountMItems(MenuHandle theMenu);
void CopyMask(const BitMap* srcBits, const BitMap* maskBits, const BitMap* dstBits, const Rect* srcRect, const Rect* maskRect,
    const Rect* dstRect);
void PaintRect(const Rect* r);
Boolean StillDown(void);
void SystemClick(const EventRecord* theEvent, WindowPtr theWindow);
void ObscureCursor(void);
OSErr PlotCIcon(const Rect* theRect, CIconHandle theIcon);
OSErr DisposeCIcon(CIconHandle theIcon);
void InsetRect(Rect* r, int16_t dh, int16_t dv);
Boolean SectRect(const Rect* src1, const Rect* src2, Rect* dstRect);
void FrameOval(const Rect* r);
void HideControl(ControlHandle theControl);
void ShowControl(ControlHandle theControl);
void SetControlMaximum(ControlHandle theControl, int16_t maxValue);
#define GetDblTime() (*(uint32_t*)0x02F0)
void SizeControl(ControlHandle theControl, int16_t w, int16_t h);
int32_t DeltaPoint(Point ptA, Point ptB);
void FrameRect(const Rect* r);
int16_t TextWidth(const void* textBuf, int16_t firstByte, int16_t byteCount);
TEHandle TENew(const Rect* destRect, const Rect* viewRect);
void TESetText(const void* text, int32_t length, TEHandle hTE);
void TESetSelect(int32_t selStart, int32_t selEnd, TEHandle hTE);
void TEUpdate(const Rect* rUpdate, TEHandle hTE);
void TEDelete(TEHandle hTE);
void TEDispose(TEHandle hTE);
void HideCursor(void);
void ShowCursor(void);
void GetItemMark(MenuHandle theMenu, int16_t item, int16_t* markChar);
#define GetMBarHeight() 20
void LMSetMBarHeight(int16_t h);
void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn);
RgnHandle GetGrayRgn(void);
void UnionRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn);
void SectRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn);
void PaintOne(WindowPeek window, RgnHandle clobberedRgn);
void SFGetFile(Point where, const Str255 prompt, Ptr fileFilter, int16_t numTypes, SFTypeList typeList,
    Ptr dlgHook, SFReply* reply);
void DrawControls(WindowPtr theWindow);
TEHandle TEStyleNew(const Rect* destRect, const Rect* viewRect);
void TEStyleInsert(const void* text, int32_t length, StScrpHandle hSt, TEHandle hTE);
void TESetAlignment(int16_t just, TEHandle hTE);
void TEScroll(int16_t dh, int16_t dv, TEHandle hTE);
int32_t GetResourceSizeOnDisk(Handle theResource);
void DisposeGWorld(GWorldPtr offscreenWorld);
void DisposePixPat(PixPatHandle ppat);
void SFPutFile(Point where, const Str255 prompt, const Str255 origName, Ptr dlgHook, SFReply* reply);
OSErr GetProcessInformation(const ProcessSerialNumber* PSN, ProcessInfoRecPtr info);
ControlHandle NewControl(WindowPtr theWindow, const Rect* boundsRect, ConstStr255Param title, Boolean visible,
    int16_t value, int16_t min, int16_t max, int16_t procID, int32_t refCon);
void InitGraf(void* globalPtr);
void InitWindows(void);

#define cmdKey 256
#define shiftKey 512
#define alphaLock 1024
#define optionKey 2048
#define controlKey 4096

void GetPortBounds(CGrafPtr port, Rect* rect);
void ErasePortRect(void);
void GetControlBounds(ControlHandle ctrl, Rect* rect);

static inline void MoveControlByID(int16_t id, WindowPtr dlg, int16_t dx, int16_t dy) {
  Rect r;
  ControlHandle h = GetNewControl(id, dlg);
  GetControlBounds(h, &r);
  MoveControl(h, r.left + dx, r.top + dy);
}

void InitRealmzCocoa();

#include "structs.h"
#include "variables.h"

#endif /* RealmzCocoa_h */
