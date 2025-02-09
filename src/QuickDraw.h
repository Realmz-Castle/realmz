#ifndef QuickDraw_h
#define QuickDraw_h

#include <stdint.h>
#include <stdlib.h>

#include "CGrafPort.h"
#include "FileManager.h"
#include "Types.h"

#define whiteColor 30
#define blackColor 33
#define yellowColor 69
#define redColor 205
#define cyanColor 273
#define greenColor 341
#define blueColor 409

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef Handle RgnHandle;
typedef uint32_t GWorldFlags;

typedef struct {
  Rect gdRect;
  PixMapHandle gdPMap;
} GDevice;
typedef GDevice *GDPtr, **GDHandle;
typedef int16_t QDErr;

typedef struct {
} ColorTable;

typedef ColorTable* CTabPtr;
typedef CTabPtr* CTabHandle;

typedef struct {
  uint16_t picSize;
  Rect picFrame;
  Handle data;
} Picture;
typedef Picture *PicPtr, **PicHandle;

// Since we'll only be using color graphics ports, alias GrafPort to CGrafPort,
// for simpler type manipulations. In the Classic programming environment, the two
// were nearly identical and could be safely casted to each other. While we could
// work to achieve the same parity, it's unnecessary.
typedef CGrafPort GrafPort;

typedef CGrafPort* CGrafPtr;
typedef CGrafPort** CGrafHandle; // Not part of Classic Mac OS API
typedef CGrafPtr GWorldPtr;
typedef GrafPort* GrafPtr;
typedef GrafPort** GrafHandle; // Not part of Classic Mac OS API

typedef struct {
  PixMap iconPMap;
  BitMap iconMask;
  BitMap iconBMap;
  Handle iconData;
  int16_t iconMaskData;
} CIcon;
typedef CIcon *CIconPtr, **CIconHandle;

typedef struct {
  CGrafPtr thePort;
  BitMap screenBits;

  // We internally allocate a handle for thePort, even though thePort itself is
  // not a handle, so we store the handle here. This is not part of the Classic
  // Mac OS API - they didn't intend for GrafPorts to be relocatable, but we
  // never relocate handles anyway.
  CGrafHandle default_graf_handle;
} QuickDrawGlobals;

Boolean PtInRect(Point pt, const Rect* r);
// Note: Technically the argument to InitGraf is a void*, but we type it here
// for better safety.
void InitGraf(QuickDrawGlobals* globalPtr);
void SetPort(CGrafPtr port);
void GetPort(GrafPtr* port);
PixPatHandle GetPixPat(uint16_t patID);
PicHandle GetPicture(int16_t picID);
void ForeColor(int32_t color);
void GetBackColor(RGBColor* color);
void GetForeColor(RGBColor* color);
void BackColor(int32_t color);
void TextFont(uint16_t font);
void TextMode(int16_t mode);
void TextSize(uint16_t size);
void TextFace(int16_t face);
void RGBBackColor(const RGBColor* color);
void RGBForeColor(const RGBColor* color);
CIconHandle GetCIcon(uint16_t iconID);
void BackPixPat(PixPatHandle ppat);
void MoveTo(int16_t h, int16_t v);
void InsetRect(Rect* r, int16_t dh, int16_t dv);
void PenPixPat(PixPatHandle ppat);
void GetGWorld(CGrafPtr* port, GDHandle* gdh);
PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld);
QDErr NewGWorld(GWorldPtr* offscreenGWorld, int16_t pixelDepth, const Rect* boundsRect, CTabHandle cTable,
    GDHandle aGDevice, GWorldFlags flags);
void SetGWorld(CGrafPtr port, GDHandle gdh);
void DisposeGWorld(GWorldPtr offscreenWorld);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // QuickDraw_h
