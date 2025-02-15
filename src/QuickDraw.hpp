#ifndef QuickDraw_hpp
#define QuickDraw_hpp

#include "QuickDraw.h"

#include <memory>

#include <SDL3/SDL_pixels.h>

#include "GraphicsCanvas.hpp"
#include <phosg/Strings.hh>

Rect rect_from_reader(phosg::StringReader& data);

uint32_t rgba8888_for_rgb_color(const RGBColor& color);
SDL_Color sdl_color_for_rgb_color(const RGBColor& color);

uint32_t GetBackColorRGBA8888();
uint32_t GetForeColorRGBA8888();
SDL_Color GetBackColorSDL();
SDL_Color GetForeColorSDL();

// Adds a canvas to the global lookup, so that they can be accessed directly via
// a CGrafPtr.
void register_canvas(CGrafPtr port, std::shared_ptr<GraphicsCanvas> canvas);
void deregister_canvas(CGrafPtr port);
std::shared_ptr<GraphicsCanvas> lookup_canvas(CGrafPtr port);

// Drawing functions which operate on the current port, defined globally as qd.thePort
// and associated with a GraphicsCanvas which performs the draw calls via SDL.
void draw_rect(const Rect& dispRect);
void draw_rgba_picture(void* pixels, int w, int h, const Rect& rect);
void set_draw_color(const RGBColor& color);
void draw_line(const Point& start, const Point& end);
// Draws the specified text at the current pen location and with the current
// font characteristics stored in the window's port
void draw_text(const std::string& text);
void render_current_canvas(const SDL_FRect* rect);

#endif // QuickDraw_hpp
