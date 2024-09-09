#pragma once

#include "../MenuManager.hpp"
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

void MCSync(std::shared_ptr<MenuList> menuList);

#ifdef __cplusplus
}
#endif