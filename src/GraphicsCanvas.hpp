#pragma once

#include "QuickDraw.h"
#include "SDLHelpers.hpp"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include "Types.h"

std::string replace_param_text(const std::string& text);
sdl_renderer_shared get_dummy_renderer();
int draw_text(
    SDL_Renderer* sdlRenderer,
    const std::string& text,
    int16_t x,
    int16_t y,
    int16_t font_id,
    float pt,
    int16_t face);

// A context for all drawing and rendering operations, either attached to an SDL_Window or
// in memory. Each GraphicsCanvas may own an SDL_Surface, may own an SDL_Texture, and has either a
// shared reference to the parent SDL_Window or owns its own independent SDL_Renderer. After construction,
// each GraphicsCanvas must invoke `init()` in order to establish these members and report any errors.
//
// The renderer for a given window must be shared among all GraphicsCanvases which might eventually
// draw to the window. For example, each DialogItem and Text field will have its own GraphicsCanvas upon
// which to draw itself and save its state to its texture. Then, when the window is redrawn, each
// item's GraphicsCanvas will use the shared renderer to draw its saved texture to the window.
class GraphicsCanvas {
private:
  int width, height;
  bool is_initialized = false;
  sdl_surface_ptr sdlSurface;
  sdl_texture_ptr sdlTexture;
  sdl_renderer_ptr sdlRenderer;
  sdl_window_shared sdlWindow;
  CGrafPtr port;

public:
  GraphicsCanvas();
  GraphicsCanvas(int width, int height, CGrafPtr port);
  GraphicsCanvas(sdl_window_shared window, CGrafPtr port);
  GraphicsCanvas(sdl_window_shared window, const Rect& rect, CGrafPtr port);
  GraphicsCanvas(const Rect& rect, CGrafPtr port);
  ~GraphicsCanvas() = default;
  GraphicsCanvas(const GraphicsCanvas& gc) = delete; // Can't copy due to unique_ptr members
  GraphicsCanvas(GraphicsCanvas&& gc);

  GraphicsCanvas& operator=(GraphicsCanvas&& gc);

  const CGrafPtr& get_port() const;
  bool init();
  void clear();
  void clear_window();
  void render(const SDL_FRect*);
  void sync();
  void set_draw_color(const RGBColor& color);
  void draw_rgba_picture(void* pixels, int w, int h, const Rect& rect);
  bool draw_text(const std::string& text, const Rect& dispRect);
  // Draws the specified text when the display bounds are unknown. Updates the port's pen location
  // after the draw to be immediately to the right of the drawn text.
  void draw_text(const std::string& text);
  // Uses the GraphicsCanvas' port settings to draw the specified text to a dummy renderer and
  // return its rendered width in pixels.
  int measure_text(const std::string& text);
  void draw_rect(const Rect& dispRect);
  void draw_line(const Point& start, const Point& end);
  void draw_background(sdl_window_shared sdlWindow, PixPatHandle bkPixPat);

private:
  bool init_renderer(GraphicsCanvas& self);
  SDL_Renderer* start_draw(const GraphicsCanvas& self);
  void end_draw(const GraphicsCanvas& self);
  void clear(SDL_Renderer* renderer);
};