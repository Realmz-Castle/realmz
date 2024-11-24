#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////
// Memory Manager API:

Handle NewHandle(Size size);
Handle NewHandleClear(Size size);
void DisposeHandle(Handle handle);
Size GetHandleSize(Handle h);
void SetHandleSize(Handle h, Size newSize);

void HNoPurge(Handle h);
OSErr MemError(void);
void HPurge(Handle h);
void HLockHi(Handle h);
void HLock(Handle h);
void HUnlock(Handle h);

////////////////////////////////////////////////////////////////////////////////
// Not part of Classic Mac OS API, but convenient for our implementation:

Handle NewHandleWithData(const void* data, size_t size);
// This function replaces a handle with the contents of another handle. After
// it returns, the original data associated with dest is deallocated and src is
// not valid (its block was moved into dest).
void ReplaceHandle(Handle dest, Handle src);

////////////////////////////////////////////////////////////////////////////////
// Generic memory functions, which don't actually use the Memory Manager but
// are somewhat relevant here:

void BlockMove(const void* srcPtr, void* destPtr, Size byteCount);
void BlockMoveData(const void* srcPtr, void* destPtr, Size byteCount);
int16_t HiWord(int32_t x);
int16_t LoWord(int32_t x);
void BitClr(void* bytePtr, uint32_t bitNum);
void BitSet(void* bytePtr, int32_t bitNum);
Boolean BitTst(const void* bytePtr, int32_t bitNum);
int16_t Random(void);

#ifdef __cplusplus
}
#endif // __cplusplus
