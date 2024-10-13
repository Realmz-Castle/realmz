#ifndef QuickDraw_h
#define QuickDraw_h

#include <stdint.h>
#include <stdlib.h>

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

typedef struct {
  Ptr baseAddr;
  int16_t rowBytes;
  Rect bounds;
} BitMap;

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
  uint16_t picSize;
  Rect picFrame;
  Handle data;
} Picture;
typedef Picture *PicPtr, **PicHandle;

typedef struct {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
} RGBColor;

typedef struct {
  BitMap portBits;
} GrafPort;

typedef struct {
  Rect portRect;
  int16_t txFont;
  Style txFace;
  int16_t txMode;
  int16_t txSize;
  PixMapHandle portPixMap;

  int32_t fgColor;
  int32_t bgColor;
  RGBColor rgbFgColor;
  RGBColor rgbBgColor;
} CGrafPort;
typedef CGrafPort* CGrafPtr;
typedef CGrafPtr GWorldPtr;
typedef GrafPort* GrafPtr;

typedef struct {
  PixMap iconPMap;
  BitMap iconMask;
  BitMap iconBMap;
  Handle iconData;
  int16_t iconMaskData;
} CIcon;
typedef CIcon *CIconPtr, **CIconHandle;

void InitGraf(void* globalPtr);
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

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // QuickDraw_h
