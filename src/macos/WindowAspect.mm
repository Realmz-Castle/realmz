#include "WindowAspect.h"

#import <Cocoa/Cocoa.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>

// Clears the 4:3 lock via contentResizeIncrements, which resets contentAspectRatio
// as a documented side effect. SDL_SetWindowAspectRatio(0, 0) must not be used here:
// it leaves the NSWindow in a state that crashes AppKit's fullscreen exit
// (AppKit bug, SDL #notourbug; see SDL #14229).
extern "C" void MacResetWindowAspect(struct SDL_Window* window) {
  NSWindow* nswindow = (__bridge NSWindow*)SDL_GetPointerProperty(
      SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
  if (nswindow) {
    [nswindow setContentResizeIncrements:NSMakeSize(1, 1)];
  }
}
