#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

Handle NewHandle(Size size);
Handle NewHandleClear(Size size);
void DisposeHandle(Handle handle);
Size GetHandleSize(Handle h);
void SetHandleSize(Handle h, Size newSize);

void HNoPurge(Handle h);
OSErr MemError(void);
void HPurge(Handle h);

// Not part of Classic Mac OS API, but convenient for our implementation
Handle NewHandleWithData(const void* data, size_t size);
// This function replaces a handle with the contents of another handle. After
// it returns, the original data associated with dest is deallocated and src is
// not valid (its block was moved into dest).
void ReplaceHandle(Handle dest, Handle src);

#ifdef __cplusplus
}
#endif // __cplusplus
