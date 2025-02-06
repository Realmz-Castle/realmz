#pragma once

#include "Types.h"

enum {
  normal = 0x00,
  bold = 0x01,
  italic = 0x02,
  underline = 0x04,
  outline = 0x08,
  shadow = 0x10,
  condense = 0x20,
  extend = 0x40,
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void ParamText(ConstStr255Param param0, ConstStr255Param param1,
    ConstStr255Param param2, ConstStr255Param param3);

#ifdef __cplusplus
} // extern "C"
#endif