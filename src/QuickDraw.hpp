#pragma once

#include "QuickDraw.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <memory>
#include <phosg/Image.hh>
#include <phosg/Strings.hh>
#include <resource_file/BitmapFontRenderer.hh>

struct CCGrafPort : public CGrafPort {
public:
  // NOTE: All data members in this struct must be public, so the struct will
  // be standard layout and therefore compatible with C code. This restriction
  // does not apply to non-virtual member functions, however, since they don't
  // affect the memory layout, but we don't have any private functions anyway.
  phosg::PrefixedLogger log;
  phosg::ImageRGBA8888N data;
  bool is_window;

  static std::unordered_set<const CCGrafPort*> all_ports;
  static CCGrafPort* as_port(void* ptr); // Returns null if ptr is not a CCGrafPort
  static const CCGrafPort* as_port(const void* ptr); // Returns null if ptr is not a CCGrafPort

  CCGrafPort();
  explicit CCGrafPort(const Rect& bounds, bool is_window = false);
  ~CCGrafPort();

  CCGrafPort(const CCGrafPort&) = delete;
  CCGrafPort(CCGrafPort&&) = delete;
  CCGrafPort& operator=(const CCGrafPort&) = delete;
  CCGrafPort& operator=(CCGrafPort&&) = delete;

  inline Rect to_local_space(const Rect& r) const {
    return Rect{
        .top = static_cast<int16_t>(r.top - this->portRect.top),
        .left = static_cast<int16_t>(r.left - this->portRect.left),
        .bottom = static_cast<int16_t>(r.bottom - this->portRect.top),
        .right = static_cast<int16_t>(r.right - this->portRect.left),
    };
  }
  inline Point to_local_space(const Point& p) const {
    return Point{
        .v = static_cast<int16_t>(p.v - this->portRect.top),
        .h = static_cast<int16_t>(p.h - this->portRect.left),
    };
  }
  inline Rect to_global_space(const Rect& r) const {
    return Rect{
        .top = static_cast<int16_t>(r.top + this->portRect.top),
        .left = static_cast<int16_t>(r.left + this->portRect.left),
        .bottom = static_cast<int16_t>(r.bottom + this->portRect.top),
        .right = static_cast<int16_t>(r.right + this->portRect.left),
    };
  }
  inline Point to_global_space(const Point& p) const {
    return Point{
        .v = static_cast<int16_t>(p.v + this->portRect.top),
        .h = static_cast<int16_t>(p.h + this->portRect.left),
    };
  }

  inline size_t get_width() const {
    return this->data.get_width();
  }
  inline size_t get_height() const {
    return this->data.get_height();
  }

  void resize(size_t w, size_t h);

  void erase_rect(const Rect& rect);
  void draw_ga11_data(const void* pixels, int w, int h, const Rect& rect);
  void draw_rgba8888_data(const void* pixels, int w, int h, const Rect& rect);
  void draw_decoded_pict_from_handle(PicHandle pict, const Rect& rect);
  bool draw_text(const std::string& text, const Rect& dispRect);
  // Draws the specified text when the display bounds are unknown. Updates the port's pen location
  // after the draw to be immediately to the right of the drawn text.
  void draw_text(const std::string& text);
  // Returns the rendered width of the given text, in pixels
  int measure_text(const std::string& text);
  void draw_rect(const Rect& dispRect);
  void draw_oval(const Rect& dispRect);
  void draw_line(const Point& start, const Point& end); // Does not affect pnLoc
  void draw_line_to(const Point& end); // pnLoc is start, and is updated to end after this call
  void draw_background_ppat();
  void draw_background_ppat(const Rect& rect); // EraseRect
  void copy_from(const CCGrafPort& src, const Rect& srcRect, const Rect& dstRect, int16_t mode);

  inline std::string ref() const {
    return std::format("P-{:016X}", reinterpret_cast<intptr_t>(this));
  }

protected:
  bool draw_text_ttf(TTF_Font* font, const std::string& processed_text, const Rect& rect);
  bool draw_text_bitmap(const ResourceDASM::BitmapFontRenderer& renderer, const std::string& text, const Rect& rect);
};

CCGrafPort& get_default_port();

Rect rect_from_reader(phosg::StringReader& data);

inline uint32_t rgba8888_for_rgb_color(const RGBColor& color) {
  return (
      (((color.red / 0x0101) & 0xFF) << 24) |
      (((color.green / 0x0101) & 0xFF) << 16) |
      (((color.blue / 0x0101) & 0xFF) << 8) |
      0xFF);
}
inline SDL_Color sdl_color_for_rgb_color(const RGBColor& color) {
  return SDL_Color{
      static_cast<uint8_t>(color.red / 0x0101),
      static_cast<uint8_t>(color.green / 0x0101),
      static_cast<uint8_t>(color.blue / 0x0101),
      0xFF};
}

uint32_t GetBackColorRGBA8888();
uint32_t GetForeColorRGBA8888();
SDL_Color GetBackColorSDL();
SDL_Color GetForeColorSDL();

CCGrafPort& current_port();
