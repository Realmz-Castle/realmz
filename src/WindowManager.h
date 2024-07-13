#ifndef WindowManager_h
#define WindowManager_h

#include "ResourceManager.h"
#include "ResourceTypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void WindowManager_Init(void);
WindowPtr WindowManager_CreateNewWindow(Rect bounds, char* title, bool visible, int procID, WindowPtr behind,
    bool goAwayFlag, int32_t refCon, uint16_t numItems, ResourceManager_DialogItem* dItems);
void WindowManager_DrawDialog(WindowPtr theWindow);
bool WindowManager_WaitNextEvent(EventRecord* theEvent);
void WindowManager_MoveWindow(WindowPtr theWindow, uint16_t hGlobal, uint16_t vGlobal, bool front);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // WindowManager_h
