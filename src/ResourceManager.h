#ifndef ResourceManager_h
#define ResourceManager_h

#include <stdint.h>
#include <stdlib.h>

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define resNotFound -192

typedef struct {
  int32_t top;
  int32_t left;
  int32_t bottom;
  int32_t right;
} ResourceManager_Rect;

typedef struct {
  uint16_t pixelSize;
  ResourceManager_Rect bounds;
} ResourceManager_PixMap;

typedef struct {
  unsigned char pat[8];
} ResourceManager_Pattern;

typedef struct {
  uint16_t patType;
  ResourceManager_PixMap rmPatMap;
  const char** patData;
  char** patXData;
  int16_t patXValid;
  char** patXMap;
  ResourceManager_Pattern rmPat1Data;
} ResourceManager_PixPat;

typedef struct {
  uint16_t picSize;
  ResourceManager_Rect picFrame;
  const void* data;
} ResourceManager_Picture;

typedef struct {
  uint32_t freq;
  const void* data;
  size_t len;
} ResourceManager_Sound;

typedef struct {
  ResourceManager_Rect portBounds;
  int16_t procID;
  bool visible;
  bool dismissable;
  uint32_t refCon;
  char windowTitle[256];
  uint16_t posSpec;
} ResourceManager_Window;

typedef struct {
  ResourceManager_Rect bounds;
  int16_t wDefID;
  bool visible;
  bool dismissable;
  uint32_t refCon;
  int16_t ditlID;
} ResourceManager_Dialog;

typedef struct {
  ResourceManager_Rect dispRect;
  bool enabled;
  ResourceManager_Picture p;
} ResourceManager_DialogItemPict;
typedef union {
  ResourceManager_DialogItemPict pict;
} ResourceManager_DialogItemType;
typedef struct {
  enum ResourceManager_DIALOG_ITEM_TYPE {
    DIALOG_ITEM_TYPE_PICT,
  } type;
  ResourceManager_DialogItemType dialogItem;
} ResourceManager_DialogItem;

void* ResourceManager_GetResource(int32_t theType, int16_t theID);
int16_t ResourceManager_OpenResFile(const char filename[256], signed char permission);
void ResourceManager_UseResFile(int16_t reference_number);
int16_t ResourceManager_CurResFile();

void ResourceManager_get_Str255_from_strN(char* out, int16_t res_id, uint16_t index);
ResourceManager_PixPat ResourceManager_get_ppat_resource(int16_t patID);
ResourceManager_Picture ResourceManager_get_pict_resource(int16_t picID);
ResourceManager_Window ResourceManager_get_wind_resource(int16_t windowID);
ResourceManager_Dialog ResourceManager_get_dlog_resource(int16_t dialogID);
ResourceManager_Sound ResourceManager_get_snd_resource(int16_t soundID);
uint16_t ResourceManager_get_ditl_resources(int16_t ditlID, ResourceManager_DialogItem** items);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ResourceManager_hh
