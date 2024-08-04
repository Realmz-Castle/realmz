#ifndef FileManager_h
#define FileManager_h

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
  fsCurPerm = 0,
  fsRdPerm = 1,
  fsWrPerm = 2,
  fsRdWrPerm = 3,
  fsRdWrShPerm = 4,
};

enum {
  fnfErr = -43
};

typedef struct {
  int16_t vRefNum;
  int32_t parID;
  Str63 name;
} FSSpec;
typedef FSSpec* FSSpecPtr;
typedef FSSpecPtr* FSSpecHandle;

typedef struct {
  OSType fdType;
  OSType fdCreator;
  uint16_t fdFlags;
  Point fdLocation;
  uint16_t fdFldr;
} FInfo;

OSErr GetVInfo(int16_t drvNum, StringPtr volName, int16_t* vRefNum, int32_t* freeBytes);
void GetFInfo(const Str63 fName, int16_t vRefNum, FInfo* fInfo);
OSErr FSpGetFInfo(const FSSpec* spec, FInfo* fndrInfo);
OSErr FSpSetFInfo(const FSSpec* spec, const FInfo* fndrInfo);
OSErr FSpDelete(const FSSpec* spec);

OSErr FSMakeFSSpec(int16_t vRefNum, int32_t dirID, ConstStr255Param fileName, FSSpecPtr spec);
OSErr FindFolder(int16_t vRefNum, OSType folderType, Boolean createFolder, int16_t* foundVRefNum, int32_t* foundDirID);

FILE* mac_fopen(const char* filename, const char* mode);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // FileManager_h
