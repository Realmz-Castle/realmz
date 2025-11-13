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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EventManager.h"
#include "FileManager.h"
#include "Font.h"
#include "MemoryManager.h"
#include "MenuManager-C-Interface.h"
#include "QuickDraw.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Types.h"
#include "WindowManager.h"
#include "convert.h"

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
#define QDFlushPortBuffer(x, y)
#define GetPortPixMap(x) &(((CGrafPtr)(x))->portPixMap);
#define GetPortTextFont(x) ((x)->txFont)
#define GetPortTextFace(x) ((x)->txFace)
#define GetPortTextSize(x) ((x)->txSize)
#define GetPortTextMode(x) ((x)->txMode)

#define GetNewWindow GetNewCWindow

#define suspendResumeMessage 0x01

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

  int i;
  for (i = 0; i < len; i++) {
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

typedef Handle GammaTblHandle;
typedef Handle SndListHandle;
typedef DialogPtr DialogRef;
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
  Boolean good;
  Str63 fName;
  int16_t vRefNum;
} SFReply;
typedef OSType SFTypeList[4];

typedef struct {
  uint32_t processInfoLength;
  StringPtr processName;
  FSSpecPtr processAppSpec;
} ProcessInfoRec;
typedef ProcessInfoRec* ProcessInfoRecPtr;

void LocalToGlobal(Point* pt);
PixPatHandle GetPixPat(uint16_t patID);
GDHandle GetMainDevice(void);

void SysBeep(uint16_t duration);
#define charCodeMask 0x000000FF
void SelectDialogItemText(DialogPtr theDialog, int16_t itemNo, int16_t strtSel, int16_t endSel);
RgnHandle NewRgn(void);
void RectRgn(RgnHandle rgn, const Rect* r);
int32_t DragGrayRgn(RgnHandle theRgn, Point startPt, const Rect* boundsRect, const Rect* slopRect,
    int16_t axis, Ptr actionProc);
void ExitToShell(void);
OSErr SetDepth(GDHandle aDevice, uint16_t depth, uint16_t whichFlags, uint16_t flags);
OSErr GetDefaultOutputVolume(uint32_t* level);
void GetSoundVol(short* x);
OSErr SetDefaultOutputVolume(uint32_t level);
#define SetSoundVol SetDefaultOutputVolume
void TextFont(uint16_t font);
int32_t MenuKey(int16_t ch);
void HiliteMenu(int16_t menuID);
void TextSize(uint16_t size);
GDHandle GetGDevice(void);
Boolean LockPixels(PixMapHandle pm);
void UnlockPixels(PixMapHandle pm);
PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld);
Size MaxMem(Size* grow);
void BackPixPat(PixPatHandle ppat);
void TextMode(int16_t mode);
void SetItemIcon(MenuHandle theMenu, int16_t item, int16_t iconIndex);
void OffsetRect(Rect* r, uint16_t dh, uint16_t dv);

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
void BeginUpdate(WindowPtr theWindow);
void EndUpdate(WindowPtr theWindow);
void SetPt(Point* pt, int16_t h, int16_t v);
int16_t DIBadMount(Point where, int32_t evtMessage);

#define GetDialogWindow(pDialog) (pDialog)
#define GetWindowPort(x) (x)
#define BitAnd(x, y) ((x) & (y))
int16_t OpenDeskAcc(ConstStr255Param deskAccName);
void SetItemMark(MenuHandle theMenu, int16_t item, int16_t markChar);
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
int32_t GetResourceSizeOnDisk(Handle theResource);
void SFPutFile(Point where, const Str255 prompt, const Str255 origName, Ptr dlgHook, SFReply* reply);
OSErr GetProcessInformation(const ProcessSerialNumber* PSN, ProcessInfoRecPtr info);
void InitWindows(void);

#define cmdKey 256
#define shiftKey 512
#define alphaLock 1024
#define optionKey 2048
#define controlKey 4096

void GetPortBounds(CGrafPtr port, Rect* rect);
void ErasePortRect(void);

void InitRealmzCocoa();

#include "structs.h"
#include "variables.h"

#endif /* RealmzCocoa_h */
