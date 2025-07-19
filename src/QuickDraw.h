#ifndef QuickDraw_h
#define QuickDraw_h

#include <stdint.h>
#include <stdlib.h>

#include "Types.h"

#define whiteColor 30
#define blackColor 33
#define yellowColor 69
#define redColor 205
#define cyanColor 273
#define greenColor 341
#define blueColor 409

#define GetPortBitMapForCopyBits(x) ((BitMap*)x)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef Handle RgnHandle;
typedef Handle CCrsrHandle;
typedef uint32_t GWorldFlags;

typedef struct {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
} RGBColor;

typedef struct {
  // Note: The original structure has more than this, but only this field is
  // necessary in our implementation since we don't support monochrome drawing
  Rect bounds;
} BitMap;

/*
 A pixel map, which is defined by a data structure of type PixMap, contains information about the
 dimensions and contents of a pixel image, as well as information on the image’s storage format,
 depth, resolution, and color usage.

 Imaging with Quickdraw (4-46 Color QuickDraw Reference)
 Imaging with Quickdraw (4-118 Summary of Color Quickdraw)
 */
typedef struct {
  uint16_t pixelSize; // physical bits per pixel
  Rect bounds; // boundary rectangle
} PixMap;
typedef PixMap *PixMapPtr, **PixMapHandle;

// Imaging With Quickdraw (3-152 Summary of Quickdraw Drawing)
typedef struct {
  unsigned char pat[8];
} Pattern;

// Imaging With Quickdraw (4-120 Summary of Color Quickdraw)
typedef struct {
  uint16_t patType; /* pattern type */
  PixMapHandle patMap; /* PixMap structure for pattern */
  Handle patData; /* pixel-image defining pattern */
  Handle patXData; /* expanded pattern image */
  int16_t patXValid; /* for expanded pattern data */
  Handle patXMap; /* handle to expanded pattern data */
  Pattern pat1Data; /* a bit pattern for a GrafPort structure */
} PixPat;
typedef PixPat *PixPatPtr, **PixPatHandle;

typedef struct {
  // NOTE: This structure must only be constructed and destroyed in C++ code as
  // part of a CCGrafPort. Multiple places in our implementation assume that
  // every CGrafPort is actually a CCGrafPort.
  BitMap portBits; // Unused in our implementation! See CCGrafPort instead
  Rect portRect;
  int16_t txFont;
  Style txFace;
  int16_t txMode;
  int16_t txSize;
  Point pnLoc;
  Point pnSize;
  int16_t pnMode;
  PixMapHandle portPixMap;
  PixPatHandle pnPixPat;
  PixPatHandle bkPixPat;
  int32_t fgColor;
  int32_t bgColor;
  RGBColor rgbFgColor;
  RGBColor rgbBgColor;
} CGrafPort;

typedef struct {
  Rect gdRect;
  PixMapHandle gdPMap;
} GDevice;
typedef GDevice *GDPtr, **GDHandle;
typedef int16_t QDErr;

typedef struct {
  size_t num_colors;
  Handle colors; // RGBColor[num_colors]
} ColorTable;

typedef ColorTable* CTabPtr;
typedef CTabPtr* CTabHandle;

typedef struct {
  uint16_t picSize;
  Rect picFrame;
  uint8_t command_data[0];
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

typedef Handle TEHandle;

typedef struct {
  PixMap iconPMap;
  BitMap iconBMap;
  Handle iconData; // ImageRGBA8888N::DataT (uint32_t)
  Handle bitmapData; // ImageGA11::DataT (uint8_t/4)
} CIcon;
typedef CIcon *CIconPtr, **CIconHandle;

typedef struct {
  CGrafPtr thePort;
  BitMap screenBits;
} QuickDrawGlobals;

// Global struct holding the current graphics port.
// Moved from variables.h to avoid c++ keyword conflicts in prototype.h
extern QuickDrawGlobals qd;

void GetPortBounds(CGrafPtr port, Rect* rect);
void ErasePortRect();

Boolean PtInRect(Point pt, const Rect* r);
// Note: Technically the argument to InitGraf is a void*, but we type it here
// for better safety.
void InitGraf(QuickDrawGlobals* globalPtr);
void SetPort(CGrafPtr port);
void GetPort(GrafPtr* port);
PixPatHandle GetPixPat(uint16_t patID);
void DisposePixPat(PixPatHandle ppat);
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
OSErr DisposeCIcon(CIconHandle handle);
OSErr PlotCIcon(const Rect* theRect, CIconHandle theIcon);
OSErr PlotCIconBitmap(const Rect* theRect, CIconHandle theIcon); // Extension (draws monochrome bitmap instead of color icon)
void BackPixPat(PixPatHandle ppat);
void MoveTo(int16_t h, int16_t v);
void InsetRect(Rect* r, int16_t dh, int16_t dv);
void PenPixPat(PixPatHandle ppat);
void PenSize(int16_t width, int16_t height);
void PenMode(int16_t mode);
void GetGWorld(CGrafPtr* port, GDHandle* gdh);
PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld);
QDErr NewGWorld(GWorldPtr* offscreenGWorld, int16_t pixelDepth, const Rect* boundsRect, CTabHandle cTable,
    GDHandle aGDevice, GWorldFlags flags);
void SetGWorld(CGrafPtr port, GDHandle gdh);
void DisposeGWorld(GWorldPtr offscreenWorld);
void DrawString(ConstStr255Param s);
int16_t TextWidth(const void* textBuf, int16_t firstByte, int16_t byteCount);
void DrawPicture(PicHandle myPicture, const Rect* dstRect);
void LineTo(int16_t h, int16_t v);
void FrameOval(const Rect* r);
void CopyBits(const BitMap* srcBits, BitMap* dstBits, const Rect* srcRect, const Rect* dstRect, int16_t mode,
    RgnHandle maskRgn);
void CopyMask(const BitMap* srcBits, const BitMap* maskBits, BitMap* dstBits, const Rect* srcRect, const Rect* maskRect,
    const Rect* dstRect);
void ScrollRect(const Rect* r, int16_t dh, int16_t dv, RgnHandle updateRgn);
void EraseRect(const Rect* r);

CCrsrHandle GetCCursor(uint16_t crsrID);
void SetCCursor(CCrsrHandle cCrsr);
void DisposeCCursor(CCrsrHandle cCrsr);
void ObscureCursor(void);
void HideCursor(void);
void ShowCursor(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // QuickDraw_h
