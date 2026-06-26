#include "WindowAspect.h"

#import <Cocoa/Cocoa.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>

// The following is necessary to clear lock and free-resize the window.
// SDL_SetWindowAspectRatio(0, 0) crashes via AppKit on next window frame event.
// Switching to resize by 1x1 increments drops the lock. See SDL #14229
extern "C" void MacResetWindowAspect(struct SDL_Window* window) {
  NSWindow* nswindow = (__bridge NSWindow*)SDL_GetPointerProperty(
      SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
  if (nswindow) {
    [nswindow setContentResizeIncrements:NSMakeSize(1, 1)];
  }
}
