#include "GraphicsCanvas.hpp"

#include "Font.hpp"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <phosg/Strings.hh>
#include <resource_file/BitmapFontRenderer.hh>
#include <variant>

static phosg::PrefixedLogger canvas_log("[GraphicsCanvas] ");

static std::unordered_map<int16_t, TTF_Font*> tt_fonts_by_id;
static std::unordered_map<int16_t, ResourceDASM::BitmapFontRenderer> bm_renderers_by_id;

// Used for text measuring
static sdl_renderer_shared dummy_renderer{};
sdl_renderer_shared get_dummy_renderer() {
  if (dummy_renderer == nullptr) {
    SDL_Surface* surface = SDL_CreateSurface(1000, 1000, SDL_PIXELFORMAT_RGBA32);
    dummy_renderer = sdl_make_shared(SDL_CreateSoftwareRenderer(surface));
  }
  return dummy_renderer;
}

static inline uint32_t rgba8888_for_rgb_color(const RGBColor& color) {
  return (
      (((color.red / 0x0101) & 0xFF) << 24) |
      (((color.green / 0x0101) & 0xFF) << 16) |
      (((color.blue / 0x0101) & 0xFF) << 8) |
      0xFF);
}

static inline SDL_Color sdl_color_for_rgb_color(const RGBColor& color) {
  return SDL_Color{
      static_cast<uint8_t>(color.red / 0x0101),
      static_cast<uint8_t>(color.green / 0x0101),
      static_cast<uint8_t>(color.blue / 0x0101),
      0xFF};
}

void debug_renderer_state(SDL_Renderer* renderer, const char* label) {
  SDL_Surface* surface = SDL_RenderReadPixels(renderer, NULL);
  if (surface) {
    IMG_SavePNG(surface, label);
  }
}

bool render_surface(SDL_Renderer* sdlRenderer, SDL_Surface* surface, const Rect& rect) {
  SDL_FRect text_dest;
  text_dest.x = rect.left;
  text_dest.y = rect.top;
  text_dest.w = std::min<float>(rect.right - rect.left, surface->w);
  text_dest.h = std::min<float>(rect.bottom - rect.top, surface->h);
  auto texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);

  if (!texture) {
    canvas_log.error_f("Could not render surface: {}", SDL_GetError());
    return false;
  }

  auto text_texture = sdl_make_unique(texture);

  // DEBUG: Uncomment for debugging rendered surfaces
  if (text_texture) {
    // IMG_SavePNG(surface, "render_surface.png");
  }

  SDL_FRect src_rect = {0, 0, text_dest.w, text_dest.h};

  if (!text_texture) {
    canvas_log.error_f("Failed to create texture when rendering text: {}", SDL_GetError());
    return false;
  } else if (!SDL_RenderTexture(sdlRenderer, text_texture.get(), &src_rect, &text_dest)) {
    canvas_log.error_f("Failed to render text texture: {}", SDL_GetError());
    return false;
  }
  return true;
}

void draw_rgba_picture(std::shared_ptr<SDL_Renderer> sdlRenderer, void* pixels, int w, int h, const Rect& rect) {
  auto s = sdl_make_unique(SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, 4 * w));
  if (!s) {
    canvas_log.error_f("Could not create surface: {}", SDL_GetError());
    return;
  }
  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(sdlRenderer.get(), s.get()));
  if (!t) {
    canvas_log.error_f("Could not create texture: {}", SDL_GetError());
    return;
  }
  SDL_FRect dest;
  dest.x = rect.left;
  dest.y = rect.top;
  dest.w = rect.right - rect.left;
  dest.h = rect.bottom - rect.top;
  SDL_RenderTexture(sdlRenderer.get(), t.get(), NULL, &dest);
}

void draw_rgba_picture(std::shared_ptr<SDL_Renderer> sdlRenderer, void* pixels, int w, int h, const ResourceDASM::Rect& dispRect) {
  Rect bounds{};
  copy_rect(bounds, dispRect);
  draw_rgba_picture(sdlRenderer, pixels, w, h, bounds);
}

bool draw_text_ttf(
    SDL_Renderer* sdlRenderer,
    TTF_Font* font,
    const std::string& processed_text,
    const Rect& rect,
    const SDL_Color& color) {
  auto text_surface = sdl_make_unique(TTF_RenderText_Blended_Wrapped(
      font, processed_text.data(), processed_text.size(), color, rect.right - rect.left + 50));
  if (!text_surface) {
    canvas_log.error_f("Failed to create surface when rendering text: {}", SDL_GetError());
    return false;
  } else {
    return render_surface(sdlRenderer, text_surface.get(), rect);
  }
}

bool render_img(SDL_Renderer* sdlRenderer, const phosg::Image& img, const Rect& rect) {
  auto w = img.get_width();
  auto h = img.get_height();
  auto surface = sdl_make_unique(SDL_CreateSurfaceFrom(
      w, h, SDL_PIXELFORMAT_RGBA32, const_cast<void*>(img.get_data()), 4 * img.get_width()));
  if (!surface) {
    canvas_log.error_f("Failed to create surface when rendering text: {}", SDL_GetError());
    return false;
  } else {
    return render_surface(sdlRenderer, surface.get(), rect);
  }
}

bool draw_text_bitmap(
    SDL_Renderer* sdlRenderer,
    const ResourceDASM::BitmapFontRenderer& renderer,
    const std::string& text,
    const Rect& rect,
    uint32_t rgba8888_color) {
  size_t rect_w = rect.right - rect.left;
  size_t rect_h = rect.bottom - rect.top;
  phosg::Image img(rect_w, rect_h, true);
  img.clear(0xFF00FF00); // Transparent (with magenta so it will be obvious if compositing is done incorrectly)

  std::string wrapped_text = renderer.wrap_text_to_pixel_width(text, rect_w);
  renderer.render_text(img, wrapped_text, 0, 0, rgba8888_color);

  // DEBUG: Uncomment for debugging text rendering
  // img.save("bitmap_text.png", phosg::Image::Format::PNG);
  return render_img(sdlRenderer, img, rect);
}

int draw_text(
    SDL_Renderer* sdlRenderer,
    const std::string& text,
    int16_t x,
    int16_t y,
    int16_t font_id,
    float pt,
    int16_t face,
    const RGBColor& color) {
  std::string processed_text = replace_param_text(text);

  auto font = load_font(font_id);

  if (std::holds_alternative<TTF_Font*>(font)) {
    auto tt_font = std::get<TTF_Font*>(font);
    TTF_SetFontSize(tt_font, pt);
    set_font_face(tt_font, face);
    TTF_Text* t = TTF_CreateText(NULL, tt_font, processed_text.c_str(), 0);
    int w{0}, h{0};
    TTF_GetTextSize(t, &w, &h);

    // The pen location, passed in as the x and y parameters, is at the baseline of the text, to
    // the left. So, we need to account for this in our display rect.
    // Descent is a negative number, representing the pixels below the baseline the text may extend.
    auto descent = TTF_GetFontDescent(tt_font);
    Rect r{
        static_cast<int16_t>(y - h - descent),
        x,
        static_cast<int16_t>(y - descent),
        static_cast<int16_t>(x + w)};
    TTF_DestroyText(t);
    if (!::draw_text_ttf(sdlRenderer, tt_font, processed_text, r, sdl_color_for_rgb_color(color))) {
      return -1;
    }
    return w;
  } else if (std::holds_alternative<ResourceDASM::BitmapFontRenderer>(font)) {
    auto bm_font = std::get<ResourceDASM::BitmapFontRenderer>(font);
    auto dimensions = bm_font.pixel_dimensions_for_text(processed_text);
    auto [width, height] = dimensions;

    auto img = phosg::Image{width, height, true};
    Rect rect{y, x, static_cast<int16_t>(y + height), static_cast<int16_t>(x + width)};

    bm_font.render_text(img, text, 0, 0, rgba8888_for_rgb_color(color));

    render_img(sdlRenderer, img, rect);

    return width;
  }

  canvas_log.error_f("No renderer is available for font {}; cannot render text \"{}\"", font_id, text);
  return -1;
}

// By default, GraphicsCanvases are initialized with the current port
GraphicsCanvas::GraphicsCanvas()
    : width(100),
      height(100),
      port(qd.thePort) {}

GraphicsCanvas::GraphicsCanvas(sdl_window_shared window, const Rect& rect, CGrafPtr port)
    : width(rect.right - rect.left),
      height(rect.bottom - rect.top),
      sdlWindow(window),
      port(port) {}

GraphicsCanvas::GraphicsCanvas(const CGrafPort& portRecord)
    : width(portRecord.portRect.right - portRecord.portRect.left),
      height(portRecord.portRect.bottom - portRecord.portRect.top),
      portRecord(portRecord), // Copy port parameters to local, owned record
      port(&this->portRecord) {}

GraphicsCanvas::GraphicsCanvas(GraphicsCanvas&& gc)
    : width{gc.width},
      height{gc.height},
      is_initialized{gc.is_initialized},
      sdlSurface(std::move(gc.sdlSurface)),
      sdlTexture(std::move(gc.sdlTexture)),
      sdlRenderer(std::move(gc.sdlRenderer)),
      sdlWindow(gc.sdlWindow),
      port(gc.port) {
  gc.is_initialized = false;
}

GraphicsCanvas& GraphicsCanvas::operator=(GraphicsCanvas&& gc) {
  width = gc.width;
  height = gc.height;
  is_initialized = gc.is_initialized;
  sdlSurface = std::move(gc.sdlSurface);
  sdlTexture = std::move(gc.sdlTexture);
  sdlRenderer = std::move(gc.sdlRenderer);
  sdlWindow = gc.sdlWindow;
  port = gc.port;
  gc.is_initialized = false;
  return *this;
}

CGrafPtr GraphicsCanvas::get_port() const {
  return port;
}

bool GraphicsCanvas::is_window() const {
  return sdlWindow != nullptr;
}

bool GraphicsCanvas::init() {
  if (is_initialized) {
    return true;
  }

  if (!init_renderer()) {
    return false;
  }

  auto renderer = start_draw();

  // We'll use this texture as our own backbuffer, see
  // https://stackoverflow.com/questions/63759688/sdl-renderpresent-implementation
  // The SDL wiki also shows this technique of drawing to an in-memory texture buffer
  // before rendering to the backbuffer: https://wiki.libsdl.org/SDL3/SDL_CreateTexture
  sdlTexture = sdl_make_unique(
      SDL_CreateTexture(
          renderer,
          SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET,
          width,
          height));

  if (sdlTexture.get() == nullptr) {
    canvas_log.error_f("Could not create SDL_Texture: {}", SDL_GetError());
    return false;
  }

  is_initialized = true;
  return true;
}

void GraphicsCanvas::clear() {
  auto renderer = start_draw();

  clear(renderer, NULL);

  end_draw();
}

void GraphicsCanvas::clear_rect(const Rect& rect) {
  auto renderer = start_draw();

  clear(renderer, &rect);

  end_draw();
}

void GraphicsCanvas::clear_window() {
  auto renderer = start_draw();

  // `start_draw` changes the renderer target to the local texture, so force it back to the window
  SDL_SetRenderTarget(renderer, NULL);

  clear(renderer, NULL);

  end_draw();
}

// Render this canvas' texture to the target window
void GraphicsCanvas::render(const SDL_FRect* dest) {
  if (sdlWindow) {
    auto renderer = SDL_GetRenderer(sdlWindow.get());
    SDL_RenderTexture(renderer, sdlTexture.get(), NULL, dest);
  }
}

void GraphicsCanvas::sync() {
  if (sdlWindow) {
    auto renderer = SDL_GetRenderer(sdlWindow.get());
    SDL_RenderPresent(renderer);
    SDL_SyncWindow(sdlWindow.get());
  }
}

void GraphicsCanvas::set_draw_color(const RGBColor& color) {
  auto renderer = start_draw();
  SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, SDL_ALPHA_OPAQUE);
  end_draw();
}

void GraphicsCanvas::draw_rgba_picture(void* pixels, int w, int h, const Rect& rect) {
  auto renderer = start_draw();

  auto s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, 4 * w);
  if (!s) {
    canvas_log.error_f("Could not create surface: {}", SDL_GetError());
    return;
  }

  auto dest = sdl_frect(rect);

  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(renderer, s));
  SDL_RenderTexture(renderer, t.get(), NULL, &dest);

  // pixels points to the raw image data of the resource, so we have to make sure not to accidentally
  // free it when we destroy the surface.
  s->pixels = NULL;
  SDL_DestroySurface(s);

  end_draw();
}

bool GraphicsCanvas::draw_text(
    const std::string& text,
    const Rect& dispRect) {
  auto renderer = start_draw();

  std::string processed_text = replace_param_text(text);

  auto font = load_font(port->txFont);

  bool success{false};

  if (std::holds_alternative<TTF_Font*>(font)) {
    auto tt_font = std::get<TTF_Font*>(font);
    TTF_SetFontSize(tt_font, port->txSize);
    set_font_face(tt_font, port->txFace);
    success = ::draw_text_ttf(renderer, tt_font, processed_text, dispRect, sdl_color_for_rgb_color(port->rgbFgColor));
  } else if (std::holds_alternative<ResourceDASM::BitmapFontRenderer>(font)) {
    auto bm_font = std::get<ResourceDASM::BitmapFontRenderer>(font);
    success = ::draw_text_bitmap(renderer, bm_font, processed_text, dispRect, rgba8888_for_rgb_color(port->rgbFgColor));
  }

  if (!success) {
    canvas_log.error_f("No renderer is available for font {}; cannot render text \"{}\"", port->txFont, text);
  }

  end_draw();

  return success;
}

void GraphicsCanvas::draw_text(const std::string& text) {

  auto renderer = start_draw();

  auto pen_loc = port->pnLoc;

  int width = ::draw_text(renderer, text, pen_loc.h, pen_loc.v, port->txFont, port->txSize, port->txFace, port->rgbFgColor);
  port->pnLoc.h += width;

  end_draw();
}

int GraphicsCanvas::measure_text(const std::string& text) {
  auto renderer = get_dummy_renderer();

  return ::draw_text(renderer.get(), text, 0, 0, port->txFont, port->txSize, port->txFace, port->rgbFgColor);
}

void GraphicsCanvas::draw_rect(const Rect& dispRect) {
  auto renderer = start_draw();
  auto dest = sdl_frect(dispRect);
  SDL_RenderRect(renderer, &dest);
  end_draw();
}

// Derived from https://en.wikipedia.org/wiki/Ellipse#In_Cartesian_coordinates and
// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void GraphicsCanvas::draw_oval(const Rect& dispRect) {
  auto renderer = start_draw();

  // Compute semi-major and semi-minor axes, center coordinates, and focus
  // distance from center
  auto a = (dispRect.right - dispRect.left) / 2.0;
  auto b = (dispRect.bottom - dispRect.top) / 2.0;
  int x0{}, y0{}, x{};
  int y = 0;
  bool vertical{false};

  if (a < b) {
    std::swap(a, b);
    x0 = dispRect.right - b;
    y0 = dispRect.bottom - a;
    x = b;
    vertical = true;
  } else {
    x0 = dispRect.right - a;
    y0 = dispRect.bottom - b;
    x = a;
  }

  int c = sqrt(a * a - b * b);

  // Calculate ellipse pixels in coordinates that are relative to the center,
  // then translate to actual center when drawing. Start at (x, 0), first quadrant

  // Foci
  // if (vertical) {
  //   SDL_RenderPoint(renderer, x0, y0 + c);
  //   SDL_RenderPoint(renderer, x0, y0 - c);
  // } else {
  //   SDL_RenderPoint(renderer, x0 + c, y0);
  //   SDL_RenderPoint(renderer, x0 - c, y0);
  // }

  int dx_f1{}, dx_f2{}, dy_f1{}, dy_f2{};
  while (x > 0) {
    // Mirror the pixel to quadrants 2, 3, and 4
    SDL_RenderPoint(renderer, x0 + x, y0 + y);
    SDL_RenderPoint(renderer, x0 + x, y0 - y);
    SDL_RenderPoint(renderer, x0 - x, y0 + y);
    SDL_RenderPoint(renderer, x0 - x, y0 - y);

    // Search next point to draw, starting with y+1, then y+1 and x-1, then
    // just x-1. The first one that is inside the bounds of the ellipse is our
    // next pixel to draw
    if (vertical) {
      dx_f1 = x;
      dx_f2 = x;
      dy_f1 = y - c;
      dy_f2 = y + c;
    } else {
      dx_f1 = x - c;
      dx_f2 = x + c;
      dy_f1 = y;
      dy_f2 = y;
    }

    dy_f1++;
    dy_f2++;

    if (sqrt(dx_f1 * dx_f1 + dy_f1 * dy_f1) + sqrt((dx_f2 * dx_f2 + dy_f2 * dy_f2)) < 2 * a) {
      y++;
      continue;
    }

    dx_f1--;
    dx_f2--;

    if (sqrt(dx_f1 * dx_f1 + dy_f1 * dy_f1) + sqrt((dx_f2 * dx_f2 + dy_f2 * dy_f2)) < 2 * a) {
      y++;
      x--;
      continue;
    }

    dy_f1--;
    dy_f2--;

    if (sqrt(dx_f1 * dx_f1 + dy_f1 * dy_f1) + sqrt((dx_f2 * dx_f2 + dy_f2 * dy_f2)) < 2 * a) {
      x--;
      continue;
    }
  }

  // Draw one final pixel at (0, [a|b]) and (0, [-a|-b])
  if (vertical) {
    SDL_RenderPoint(renderer, x0, y0 + a);
    SDL_RenderPoint(renderer, x0, y0 - a);
  } else {
    SDL_RenderPoint(renderer, x0, y0 + b);
    SDL_RenderPoint(renderer, x0, y0 - b);
  }

  end_draw();
}

void GraphicsCanvas::draw_line(const Point& start, const Point& end) {
  auto renderer = start_draw();
  SDL_RenderLine(
      renderer,
      static_cast<float>(start.h),
      static_cast<float>(start.v),
      static_cast<float>(end.h),
      static_cast<float>(end.v));
  end_draw();
}

void GraphicsCanvas::draw_background(sdl_window_shared sdlWindow, PixPatHandle bkPixPat) {
  auto renderer = SDL_GetRenderer(sdlWindow.get());

  PixMapHandle pmap = (*bkPixPat)->patMap;
  Rect bounds = (*pmap)->bounds;
  int w = bounds.right - bounds.left;
  int h = bounds.bottom - bounds.top;
  auto s = sdl_make_unique(SDL_CreateSurfaceFrom(
      w,
      h,
      SDL_PIXELFORMAT_RGB24,
      (*(*bkPixPat)->patData),
      3 * w));

  if (s == nullptr) {
    canvas_log.error_f("Could not create surface: {}", SDL_GetError());
    return;
  }

  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(renderer, s.get()));
  if (t == nullptr) {
    canvas_log.error_f("Could not create texture: {}", SDL_GetError());
    return;
  }

  if (!SDL_RenderTextureTiled(renderer, t.get(), NULL, 1.0, NULL)) {
    canvas_log.error_f("Could not render background texture: {}", SDL_GetError());
  }
}

void GraphicsCanvas::copy_from(GraphicsCanvas& src, const Rect& srcRect, const Rect& dstRect) {
  auto renderer = start_draw();

  int w = dstRect.right - dstRect.left;
  int h = dstRect.bottom - dstRect.top;

  auto s = sdl_make_unique(SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32));
  const auto sr = sdl_rect(srcRect);

  if (src.is_window()) {
    // Here we have to start a draw operation on the source canvas. This temporarily sets the render
    // target to the texture buffer, which we can then read from.
    auto src_renderer = src.start_draw();
    SDL_Surface* src_surface_raw;
    if (!(src_surface_raw = SDL_RenderReadPixels(src_renderer, &sr))) {
      canvas_log.error_f("Could not read pixels for copy_from: {}", SDL_GetError());
      return;
    }
    auto src_surface = sdl_make_unique(src_surface_raw);
    if (!SDL_BlitSurfaceScaled(src_surface.get(), &sr, s.get(), NULL, SDL_SCALEMODE_LINEAR)) {
      canvas_log.error_f("Could not blit surface scaled: {}", SDL_GetError());
      return;
    }
    src.end_draw();
  } else {
    if (!SDL_BlitSurfaceScaled(src.sdlSurface.get(), &sr, s.get(), NULL, SDL_SCALEMODE_LINEAR)) {
      canvas_log.error_f("Could not blit surface scaled: {}", SDL_GetError());
      return;
    }
  }

  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(renderer, s.get()));
  if (t == nullptr) {
    canvas_log.error_f("Could not create_texture: {}", SDL_GetError());
    return;
  }

  const auto dr = sdl_frect(dstRect);
  if (!SDL_RenderTexture(renderer, t.get(), NULL, &dr)) {
    canvas_log.error_f("Could not copy bits: {}", SDL_GetError());
  }

  end_draw();
}

bool GraphicsCanvas::init_renderer() {
  if (sdlWindow) {
    if (!SDL_GetRenderer(sdlWindow.get())) {
      if (!SDL_CreateRenderer(sdlWindow.get(), "opengl")) {
        canvas_log.error_f("Could not create window renderer: {}", SDL_GetError());
        return false;
      }
    }
  } else {
    sdlSurface = sdl_make_unique(SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32));
    if (sdlSurface == nullptr) {
      canvas_log.error_f("Could not create SDL_Surface for software rendering: {}", SDL_GetError());
      return false;
    }

    sdlRenderer = sdl_make_unique(SDL_CreateSoftwareRenderer(sdlSurface.get()));
    if (sdlRenderer == nullptr) {
      canvas_log.error_f("Could not create software SDL_Renderer: {}", SDL_GetError());
      return false;
    }
  }

  return true;
}

SDL_Renderer* GraphicsCanvas::start_draw() {
  if (sdlWindow) {
    auto r = SDL_GetRenderer(sdlWindow.get());
    SDL_SetRenderTarget(r, sdlTexture.get());
    return r;
  } else {
    // Software renderer, already rendering to surface
    return sdlRenderer.get();
  }
}

void GraphicsCanvas::end_draw() {
  if (sdlWindow) {
    // Restores the window's renderer's drawing target so that the next call to render the stored
    // GraphicsCanvas texture will render to the screen.
    SDL_SetRenderTarget(SDL_GetRenderer(sdlWindow.get()), NULL);
  } else {
    // For software rendering, we still have to call SDL_RenderPresent for the draw calls
    // to be flushed to the surface.
    SDL_RenderPresent(sdlRenderer.get());
  }
}

void GraphicsCanvas::clear(SDL_Renderer* renderer, const Rect* rect) {
  // Clear the texture with transparent background pixels, using no blend mode
  SDL_BlendMode blendMode;
  uint8_t r, g, b, a;

  // Save current state
  SDL_GetRenderDrawBlendMode(renderer, &blendMode);
  SDL_GetRenderDrawColor(renderer, &r, &b, &g, &a);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);

  if (rect) {
    const auto frect = sdl_frect(*rect);
    SDL_RenderFillRect(renderer, &frect);
  } else {
    SDL_RenderClear(renderer);
  }

  // Restore state
  SDL_SetRenderDrawBlendMode(renderer, blendMode);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
