#pragma once

#include "../MenuManager.hpp"
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

void MCSync(std::shared_ptr<MenuList> menuList, void (*callback)(int16_t, int16_t));

#ifdef __cplusplus
}
#endif