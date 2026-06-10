#pragma once

// Defined in WindowManager.cpp; deliberately include-free so platform menu code stays clear of heavy headers.

#ifdef __cplusplus
extern "C" {
#endif

int WM_GetScaleMode(void);
void WM_SetScaleMode(int mode);
void WM_SetWindowSize(int w, int h);
int WM_SizeFits(int w, int h);
void WM_GetWindowSize(int* w, int* h);
int WM_IsFullscreen(void);
int WM_GetAspectLocked(void);
void WM_SetAspectLocked(int locked);
int WM_GetGammaIdx(void);
void WM_SetGammaIdx(int idx);
void WM_SavePrefs(void);

#ifdef __cplusplus
}
#endif
