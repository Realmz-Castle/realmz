#include "GraphicsCanvas.hpp"

#include "Font.hpp"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <phosg/Strings.hh>
#include <resource_file/BitmapFontRenderer.hh>

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

void copy_rect(Rect& dst, const ResourceDASM::Rect& src) {
  dst.top = src.y1;
  dst.left = src.x1;
  dst.bottom = src.y2;
  dst.right = src.x2;
}

bool render_surface(SDL_Renderer* sdlRenderer, SDL_Surface* surface, const Rect& rect) {
  SDL_FRect text_dest;
  text_dest.x = rect.left;
  text_dest.y = rect.top;
  text_dest.w = std::min<float>(rect.right - rect.left, surface->w);
  text_dest.h = std::min<float>(rect.bottom - rect.top, surface->h);
  auto texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);

  if (!texture) {
    canvas_log.error("Could not render surface: %s", SDL_GetError());
    return false;
  }

  auto text_texture = sdl_make_unique(texture);

  // DEBUG: Uncomment for debugging rendered surfaces
  if (text_texture) {
    // IMG_SavePNG(surface, "render_surface.png");
  }

  if (!text_texture) {
    canvas_log.error("Failed to create texture when rendering text: %s", SDL_GetError());
    return false;
  } else if (!SDL_RenderTexture(sdlRenderer, text_texture.get(), NULL, &text_dest)) {
    canvas_log.error("Failed to render text texture: %s", SDL_GetError());
    return false;
  }
  return true;
}

void draw_rgba_picture(std::shared_ptr<SDL_Renderer> sdlRenderer, void* pixels, int w, int h, const Rect& rect) {
  auto s = sdl_make_unique(SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, 4 * w));
  if (!s) {
    canvas_log.error("Could not create surface: %s\n", SDL_GetError());
    return;
  }
  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(sdlRenderer.get(), s.get()));
  if (!t) {
    canvas_log.error("Could not create texture: %s\n", SDL_GetError());
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
      font, processed_text.data(), processed_text.size(), color, rect.right - rect.left));
  if (!text_surface) {
    canvas_log.error("Failed to create surface when rendering text: %s", SDL_GetError());
    return false;
  } else {
    return render_surface(sdlRenderer, text_surface.get(), rect);
  }
}

bool draw_text_bitmap(
    SDL_Renderer* sdlRenderer,
    const ResourceDASM::BitmapFontRenderer& renderer,
    const std::string& processed_text,
    const Rect& rect,
    uint32_t rgba8888_color) {
  size_t rect_w = rect.right - rect.left;
  size_t rect_h = rect.bottom - rect.top;
  phosg::Image img(rect_w, rect_h, true);
  img.clear(0xFF00FF00); // Transparent (with magenta so it will be obvious if compositing is done incorrectly)

  std::string wrapped_text = renderer.wrap_text_to_pixel_width(processed_text, rect_w);
  renderer.render_text(img, wrapped_text, 0, 0, rgba8888_color);

  // DEBUG: Uncomment for debugging text rendering
  // img.save("bitmap_text.png", phosg::Image::Format::PNG);

  auto text_surface = sdl_make_unique(SDL_CreateSurfaceFrom(
      rect_w, rect_h, SDL_PIXELFORMAT_RGBA32, img.get_data(), 4 * img.get_width()));
  if (!text_surface) {
    canvas_log.error("Failed to create surface when rendering text: %s", SDL_GetError());
    return false;
  } else {
    return render_surface(sdlRenderer, text_surface.get(), rect);
  }
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

  TTF_Font* tt_font = nullptr;
  ResourceDASM::BitmapFontRenderer* bm_renderer = nullptr;

  bool success = load_font(font_id, &tt_font, &bm_renderer);

  if (!success) {
    return -1;
  }

  if (tt_font != nullptr) {
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
  } else if (bm_renderer) {
    canvas_log.error("Rendering bitmap text with unknown dimensions is not supported");
    return -1;
  }

  canvas_log.error("No renderer is available for font %hd; cannot render text \"%s\"", font_id, text.c_str());
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

  if (!init_renderer(*this)) {
    return false;
  }

  auto renderer = start_draw(*this);

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
    canvas_log.error("Could not create SDL_Texture: %s", SDL_GetError());
    return false;
  }

  is_initialized = true;
  return true;
}

void GraphicsCanvas::clear() {
  auto renderer = start_draw(*this);

  clear(renderer);

  end_draw(*this);
}

void GraphicsCanvas::clear_window() {
  auto renderer = start_draw(*this);

  // `start_draw` changes the renderer target to the local texture, so force it back to the window
  SDL_SetRenderTarget(renderer, NULL);

  clear(renderer);

  end_draw(*this);
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
  auto renderer = start_draw(*this);
  SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, SDL_ALPHA_OPAQUE);
  end_draw(*this);
}

void GraphicsCanvas::draw_rgba_picture(void* pixels, int w, int h, const Rect& rect) {
  auto renderer = start_draw(*this);

  auto s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, 4 * w);
  if (!s) {
    canvas_log.error("Could not create surface: %s\n", SDL_GetError());
    return;
  }

  auto dest = sdl_frect(rect);

  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(renderer, s));
  SDL_RenderTexture(renderer, t.get(), NULL, &dest);

  // pixels points to the raw image data of the resource, so we have to make sure not to accidentally
  // free it when we destroy the surface.
  s->pixels = NULL;
  SDL_DestroySurface(s);

  end_draw(*this);
}

bool GraphicsCanvas::draw_text(
    const std::string& text,
    const Rect& dispRect) {
  auto renderer = start_draw(*this);

  std::string processed_text = replace_param_text(text);

  TTF_Font* tt_font = nullptr;
  ResourceDASM::BitmapFontRenderer* bm_renderer = nullptr;

  bool success = load_font(port->txFont, &tt_font, &bm_renderer);

  if (!success) {
    return false;
  }

  if (tt_font != nullptr) {
    TTF_SetFontSize(tt_font, port->txSize);
    set_font_face(tt_font, port->txFace);
    success = ::draw_text_ttf(renderer, tt_font, processed_text, dispRect, sdl_color_for_rgb_color(port->rgbFgColor));
  } else if (bm_renderer) {
    success = ::draw_text_bitmap(renderer, *bm_renderer, processed_text, dispRect, rgba8888_for_rgb_color(port->rgbFgColor));
  }

  if (!success) {
    canvas_log.error("No renderer is available for font %hd; cannot render text \"%s\"", port->txFont, text.c_str());
  }

  end_draw(*this);

  return success;
}

void GraphicsCanvas::draw_text(const std::string& text) {

  auto renderer = start_draw(*this);

  auto pen_loc = port->pnLoc;

  int width = ::draw_text(renderer, text, pen_loc.h, pen_loc.v, port->txFont, port->txSize, port->txFace, port->rgbFgColor);
  port->pnLoc.h += width;

  end_draw(*this);
}

int GraphicsCanvas::measure_text(const std::string& text) {
  auto renderer = get_dummy_renderer();

  return ::draw_text(renderer.get(), text, 0, 0, port->txFont, port->txSize, port->txFace, port->rgbFgColor);
}

void GraphicsCanvas::draw_rect(const Rect& dispRect) {
  auto renderer = start_draw(*this);
  auto dest = sdl_frect(dispRect);
  SDL_RenderRect(renderer, &dest);
  end_draw(*this);
}

void GraphicsCanvas::draw_line(const Point& start, const Point& end) {
  auto renderer = start_draw(*this);
  SDL_RenderLine(
      renderer,
      static_cast<float>(start.h),
      static_cast<float>(start.v),
      static_cast<float>(end.h),
      static_cast<float>(end.v));
  end_draw(*this);
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

  if (s == NULL) {
    canvas_log.error("Could not create surface: %s\n", SDL_GetError());
    return;
  }

  auto t = sdl_make_unique(SDL_CreateTextureFromSurface(renderer, s.get()));
  if (t == NULL) {
    canvas_log.error("Could not create texture: %s\n", SDL_GetError());
    return;
  }

  if (!SDL_RenderTextureTiled(renderer, t.get(), NULL, 1.0, NULL)) {
    canvas_log.error("Could not render background texture: %s", SDL_GetError());
  }
}

bool GraphicsCanvas::init_renderer(GraphicsCanvas& self) {
  if (self.sdlWindow) {
    if (!SDL_GetRenderer(self.sdlWindow.get())) {
      if (!SDL_CreateRenderer(self.sdlWindow.get(), "opengl")) {
        canvas_log.error("Could not create window renderer: %s", SDL_GetError());
        return false;
      }
    }
  } else {
    self.sdlSurface = sdl_make_unique(SDL_CreateSurface(self.width, self.height, SDL_PIXELFORMAT_RGBA32));
    if (self.sdlSurface == nullptr) {
      canvas_log.error("Could not create SDL_Surface for software rendering: %s", SDL_GetError());
      return false;
    }

    self.sdlRenderer = sdl_make_unique(SDL_CreateSoftwareRenderer(self.sdlSurface.get()));
    if (self.sdlRenderer == nullptr) {
      canvas_log.error("Could not create software SDL_Renderer: %s", SDL_GetError());
      return false;
    }
  }

  return true;
}

SDL_Renderer* GraphicsCanvas::start_draw(const GraphicsCanvas& self) {
  if (self.sdlWindow) {
    auto r = SDL_GetRenderer(self.sdlWindow.get());
    SDL_SetRenderTarget(r, self.sdlTexture.get());
    return r;
  } else {
    // Software renderer, already rendering to surface
    return self.sdlRenderer.get();
  }
}

void GraphicsCanvas::end_draw(const GraphicsCanvas& self) {
  if (self.sdlWindow) {
    // Restores the window's renderer's drawing target so that the next call to render the stored
    // GraphicsCanvas texture will render to the screen.
    SDL_SetRenderTarget(SDL_GetRenderer(self.sdlWindow.get()), NULL);
  } else {
    // For software rendering, we still have to call SDL_RenderPresent for the draw calls
    // to be flushed to the surface.
    SDL_RenderPresent(sdlRenderer.get());
  }
}

void GraphicsCanvas::clear(SDL_Renderer* renderer) {
  // Clear the texture with transparent background pixels, using no blend mode
  SDL_BlendMode blendMode;
  uint8_t r, g, b, a;

  // Save current state
  SDL_GetRenderDrawBlendMode(renderer, &blendMode);
  SDL_GetRenderDrawColor(renderer, &r, &b, &g, &a);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
  SDL_RenderClear(renderer);

  // Restore state
  SDL_SetRenderDrawBlendMode(renderer, blendMode);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}