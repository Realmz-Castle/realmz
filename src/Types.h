#ifndef Types_h
#define Types_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)

#ifndef nil
#ifdef __cplusplus
#define nil nullptr
#else
#define nil NULL
#endif
#endif

typedef uint8_t UInt8;
typedef int8_t SInt8;
typedef uint16_t UInt16;
typedef int16_t SInt16;
typedef uint32_t UInt32;
typedef int32_t SInt32;

typedef char* Ptr;
typedef Ptr* Handle;
typedef long Size;

typedef SInt16 OSErr;
typedef SInt32 OSStatus;
typedef UInt32 OSType;
typedef SInt16 ScriptCode;
typedef UInt32 ResType;
typedef unsigned char Boolean;

#define noErr 0
#define resProblem -204

typedef unsigned char Str255[256];
typedef unsigned char Str63[64];
typedef unsigned char* StringPtr;
typedef StringPtr* StringHandle;
typedef const unsigned char* ConstStr255Param;
typedef const unsigned char* ConstStr63Param;

// See also the C++ versions of these in resource_file/ResourceFormats.hh
typedef struct {
  int16_t v;
  int16_t h;
} Point;
typedef Point* PointPtr;

typedef struct {
  int16_t top;
  int16_t left;
  int16_t bottom;
  int16_t right;
} Rect;
typedef Rect* RectPtr;

typedef unsigned char Style;

typedef long (*ProcPtr)(void);

typedef struct {
  UInt32 highLongOfPSN;
  UInt32 lowLongOfPSN;
} ProcessSerialNumber;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // Types_h
