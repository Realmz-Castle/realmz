#pragma once

#include <stdint.h>

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

Handle NewHandle(Size size);
void DisposeHandle(Handle handle);
Size GetHandleSize(Handle h);
void SetHandleSize(Handle h, Size newSize);

void HNoPurge(Handle h);
OSErr MemError(void);
void HPurge(Handle h);

#ifdef __cplusplus
}
#endif // __cplusplus
