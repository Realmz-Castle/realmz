#pragma once

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef Handle MenuHandle;

Handle GetNewMBar(int16_t menuBarID);
MenuHandle GetMenuHandle(int16_t menuID);
MenuHandle GetMenu(int16_t resourceID);
void SetMenuBar(Handle menuList);
void InsertMenu(MenuHandle theMenu, int16_t beforeID);
void GetMenuItemText(MenuHandle theMenu, uint16_t item, Str255 itemString);
void DrawMenuBar(void);
void DeleteMenu(int16_t menuID);
void SetMenuItemText(MenuHandle theMenu, uint16_t item, ConstStr255Param itemString);
int32_t MenuSelect(Point startPt);
void DisableItem(MenuHandle theMenu, uint16_t item);
void EnableItem(MenuHandle theMenu, uint16_t item);
void CheckItem(MenuHandle theMenu, uint16_t item, Boolean checked);
int32_t PopUpMenuSelect(MenuHandle menu, int16_t top, int16_t left, int16_t popUpItem);
void AppendMenu(MenuHandle menu, ConstStr255Param data);
int16_t CountMItems(MenuHandle theMenu);

#ifdef __cplusplus
}
#endif