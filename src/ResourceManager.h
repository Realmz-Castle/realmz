#ifndef ResourceManager_h
#define ResourceManager_h

#include <stdint.h>
#include <stdlib.h>

#include "FileManager.h"
#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
  resNotFound = -192,
  resFNotFound = -193,
  addResFailed = -194,
};

void FSpCreateResFile(const FSSpec* spec, OSType creator, OSType fileType, ScriptCode scriptTag);
int16_t FSpOpenResFile(const FSSpec* spec, SInt8 permission);
void CloseResFile(int16_t refNum);

int16_t ResError();

UInt16 CurResFile(void);
void UseResFile(int16_t refNum);

int16_t CountResources(ResType theType);
void GetResInfo(Handle theResource, int16_t* theID, ResType* theType, Str255 name);
int16_t GetResAttrs(Handle theResource);
void SetResAttrs(Handle theResource, int16_t attrs);
Handle GetResource(ResType theType, int16_t theID);
Handle Get1Resource(ResType theType, int16_t theID);
int32_t GetResourceSizeOnDisk(Handle theResource);
void AddResource(Handle theData, ResType theType, int16_t theID, ConstStr255Param name);
void ChangedResource(Handle theResource);
void WriteResource(Handle theResource);
void RemoveResource(Handle theResource);
void UpdateResFile(int16_t refNum);

void ReleaseResource(Handle theResource);
void DetachResource(Handle theResource);

void GetIndString(Str255 out, int16_t res_id, uint16_t index);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ResourceManager_h
