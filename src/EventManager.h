#pragma once

#include "QuickDraw.h"
#include "SDL3/SDL.h"
#include "Types.h"

// See Event Manager chapter in Inside Macintosh volume 1, starting on page 241

enum {
  nullEvent = 0,
  mouseDown = 1,
  mouseUp = 2,
  keyDown = 3,
  keyUp = 4,
  autoKey = 5,
  updateEvt = 6,
  diskEvt = 7,
  activateEvt = 8,
  networkEvt = 10,
  driverEvt = 11,
  app1Evt = 12,
  app2Evt = 13,
  app3Evt = 14,
  app4Evt = 15,
  osEvt = 15,
  everyEvent = -1,
};

#define inMenuBar 1
#define inSysWindow 2
#define inContent 3

#define MAC_VK_BACKSPACE 0x33

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  // Event type (one of the event codes in the above enum)
  uint16_t what;

  // The contents of the message field depend on the event type:
  // keyDown/keyUp: char code in low byte, key code in next byte, upper word unused
  // activateEvt/updateEvt: pointer to window structure
  // diskEvt: drive number in low word, file manager result code in high word (we never generate these)
  // mouseDown/mouseUp/nullEvent: unused
  // networkEvt: handle to parameter block (we never generate these)
  // driverEvt: see driver chapter (we never generate these)
  // appEvt types: anything (application-defined)
  uint32_t message;

  // When the event occurred, in ticks (just use TickCount for this)
  uint32_t when;

  // Mouse location at time of event
  Point where;

  // Bits in the modifiers field: rosROLSCM------A
  // r = right control key down
  // o = right option key down
  // s = right shift key down
  // R = control key down
  // O = option key down
  // L = caps lock enabled
  // S = shift key down
  // C = command key down
  // M = mouse button is up (NOT DOWN, unlike the other bits!)
  // A = window activated (for activateEvt)
  uint16_t modifiers;

  // The following fields are not part of the original Classic Mac OS API
  void* window_port; // CCGrafPort pointer used to identify the window in C++
  char text[32];
} EventRecord;

uint8_t mac_vk_from_message(uint32_t message);

uint32_t TickCount(void);
uint32_t GetDblTime(void);
void SystemTask(void);
// GetCaretTime (IM1-260) not used by Realmz

Boolean GetNextEvent(int16_t mask, EventRecord* ev); // IM1-257
// EventAvail (IM1-258) not used by Realmz
void PushMenuEvent(int16_t menu_id, int16_t item_id);

void GetMouse(Point* mouseLoc); // IM1-259
void GetMouseGlobal(Point* mouseLoc); // extension (not part of original API)
Boolean Button(void); // IM1-259
Boolean StillDown(void); // IM1-259

void FlushEvents(int16_t mask, uint16_t stop_mask); // IM2-69

Boolean WaitNextEvent(int16_t mask, EventRecord* ev, uint32_t sleep, RgnHandle mouse_rgn);

void reset_mouse_state();

#ifdef __cplusplus
}
#endif
