#pragma once

#include "ResourceManager.h"
#include <SDL3/SDL.h>
#include <stdint.h>

#include "Types.h"

#define nullEvent 0
#define mouseDown 1
#define mouseUp 2
#define keyDown 3
#define keyUp 4
#define autoKey 5
#define updateEvt 6
#define diskEvt 7
#define activateEvt 8
#define osEvt 15
// Unused, but must be defined
#define networkEvt 256
#define driverEvt 257
#define app1Evt 258
#define app2Evt 259
#define app3Evt 260

// Macintosh Toolbox Essentials (Introduction to Windows 4-11)
#define plainDBox 2

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

struct GrafPort {
  BitMap portBits;
};
typedef struct GrafPort GrafPort;

typedef struct {
  Rect portRect;
  int16_t txFont;
  Style txFace;
  int16_t txMode;
  int16_t txSize;
  PixMapHandle portPixMap;
} CGrafPort;

typedef struct {
  CGrafPort port;
  int16_t windowKind;
  Boolean visible;
  Boolean goAwayFlag;
  StringHandle titleHandle;
  uint32_t refCon;

  SDL_Window* sdlWindow;
  SDL_Renderer* sdlRenderer;
  uint16_t numItems;
  ResourceManager_DialogItem* dItems;
} CWindowRecord;

typedef struct {
  uint16_t what;
  uint32_t message;
  uint32_t when;
  Point where;
  uint16_t modifiers;
} EventRecord;

#define badChannel -205
typedef struct {
  uint16_t qLength;
  SDL_AudioStream* sdlAudioStream;
} SndChannel;

typedef CGrafPort* CGrafPtr;
typedef CGrafPtr GWorldPtr;
typedef CGrafPtr CWindowPtr;
typedef GrafPort* GrafPtr;
typedef CWindowPtr WindowPtr, DialogPtr, WindowRef;
