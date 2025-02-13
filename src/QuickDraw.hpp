#ifndef QuickDraw_hpp
#define QuickDraw_hpp

#include "QuickDraw.h"

#include <SDL3/SDL_pixels.h>

#include <phosg/Strings.hh>

Rect rect_from_reader(phosg::StringReader& data);

uint32_t rgba8888_for_rgb_color(const RGBColor& color);
SDL_Color sdl_color_for_rgb_color(const RGBColor& color);

uint32_t GetBackColorRGBA8888();
uint32_t GetForeColorRGBA8888();
SDL_Color GetBackColorSDL();
SDL_Color GetForeColorSDL();

#endif // QuickDraw_hpp
