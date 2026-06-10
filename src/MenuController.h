#pragma once

#include "../MenuManager.hpp"
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

void MCSync(std::shared_ptr<MenuList> menuList, void (*callback)(int16_t, int16_t));
void MCCreatePopupMenu(void* nsWindow, std::shared_ptr<Menu> menu, std::pair<int16_t, int16_t> loc, void (*callback)(int16_t, int16_t));

int WM_GetScaleMode(void);
void WM_SetScaleMode(int mode);
void WM_SetWindowSize(int w, int h);
int WM_SizeFits(int w, int h);
void WM_GetWindowSize(int* w, int* h);
int WM_IsFullscreen(void);
int WM_GetAspectLocked(void);
void WM_SetAspectLocked(int locked);

#ifdef __cplusplus
}
#endif