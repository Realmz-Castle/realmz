#ifndef QuickDraw_h
#define QuickDraw_h

#include <stdint.h>
#include <stdlib.h>

#include "FileManager.h"
#include "Types.h"

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
} CGrafPort;
typedef CGrafPort* CGrafPtr;
typedef CGrafPtr GWorldPtr;
typedef GrafPort* GrafPtr;

PixPatHandle GetPixPat(uint16_t patID);
Picture QuickDraw_get_pict_resource(int16_t picID);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // QuickDraw_h
