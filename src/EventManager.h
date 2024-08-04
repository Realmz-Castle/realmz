#pragma once

#include "Types.h"

#define nullEvent 0
#define mouseDown 1
#define mouseUp 2
#define keyDown 3
#define keyUp 4
#define autoKey 5
#define updateEvt 6
#define diskEvt 7
#define activateEvt 8
#define osEvt 15
// Unused, but must be defined
#define networkEvt 256
#define driverEvt 257
#define app1Evt 258
#define app2Evt 259
#define app3Evt 260

typedef struct {
  uint16_t what;
  uint32_t message;
  uint32_t when;
  Point where;
  uint16_t modifiers;
} EventRecord;
