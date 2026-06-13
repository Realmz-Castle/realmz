#pragma once

#include <SDL3/SDL_surface.h>

struct PortPrefs {
  int window_w = 800;
  int window_h = 600;
  SDL_ScaleMode scale_mode = SDL_SCALEMODE_PIXELART;
  bool aspect_locked = true;
  int gamma_idx = 0; // index into kPortGammaOptions
};

PortPrefs load_port_prefs();
void save_port_prefs(const PortPrefs& prefs);
