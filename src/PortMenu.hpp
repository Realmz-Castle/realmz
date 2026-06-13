#pragma once

#include <SDL3/SDL_surface.h>

// Cross-platform definition of the contents of the "Port" menu. The menu UI is
// built natively on each platform (Cocoa on macOS, the Win32 menu API on
// Windows), but the option lists live here so both platforms stay in sync.

struct PortFilterOption {
  const char* title;
  SDL_ScaleMode mode;
};

struct PortScaleOption {
  const char* title;
  int width;
  int height;
};

inline constexpr PortFilterOption kPortFilters[] = {
    {"Pixel Art", SDL_SCALEMODE_PIXELART},
    {"Linear", SDL_SCALEMODE_LINEAR},
    {"Nearest", SDL_SCALEMODE_NEAREST},
};

inline constexpr PortScaleOption kPortScales[] = {
    {"1x", 800, 600},
    {"1.5x", 1200, 900},
    {"2x", 1600, 1200},
    {"2.5x", 2000, 1500},
    {"3x", 2400, 1800},
};

inline constexpr int kPortFilterCount = sizeof(kPortFilters) / sizeof(kPortFilters[0]);
inline constexpr int kPortScaleCount = sizeof(kPortScales) / sizeof(kPortScales[0]);

// Gamma correction options for the Color Correction submenu.
// display_gamma = 0 means off (no correction).
// Otherwise the correction applied is pow(v/255, 1.8/display_gamma) per channel,
// treating Mac source as 1.8-gamma content and converting to the given display gamma.
struct PortGammaOption {
  const char* title;
  float display_gamma;
};

inline constexpr PortGammaOption kPortGammaOptions[] = {
    {"Off",  0.0f},
    {"2.0",  2.0f},
    {"2.2",  2.2f},
    {"2.59", 2.59f},
};

inline constexpr int kPortGammaCount = sizeof(kPortGammaOptions) / sizeof(kPortGammaOptions[0]);
