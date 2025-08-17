#include "QuickDraw.hpp"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <deque>
#include <filesystem>
#include <memory>
#include <phosg/Filesystem.hh>
#include <phosg/Strings.hh>
#include <resource_file/BitmapFontRenderer.hh>
#include <resource_file/IndexFormats/Formats.hh>
#include <resource_file/QuickDrawFormats.hh>
#include <resource_file/ResourceFile.hh>
#include <resource_file/ResourceFormats.hh>
#include <resource_file/ResourceTypes.hh>
#include <resource_file/TextCodecs.hh>
#include <stdexcept>
#include <unordered_map>

#include "Font.hpp"
#include "MemoryManager.hpp"
#include "ResourceManager.h"
#include "StringConvert.hpp"
#include "Types.hpp"
#include "WindowManager.hpp"

#ifdef REALMZ_DEBUG
constexpr auto default_log_level = phosg::LogLevel::L_DEBUG;
#else
constexpr auto default_log_level = phosg::LogLevel::L_INFO;
#endif
static phosg::PrefixedLogger qd_log("[QuickDraw] ", default_log_level);

///////////////////////////////////////////////////////////////////////////////
// CCGrafPort implementation

struct DecodedPICTHeader {
  be_uint16_t size; // 0
  Rect bounds;
  be_uint16_t version_opcode; // 0x0011
  be_uint16_t version_arg; // 0x03FF
  be_uint16_t data_opcode; // 0xFFFF
  // Data follows here (RGBA8888)
};

static std::unordered_map<int16_t, TTF_Font*> tt_fonts_by_id;
static std::unordered_map<int16_t, ResourceDASM::BitmapFontRenderer> bm_renderers_by_id;

std::unordered_set<const CCGrafPort*> CCGrafPort::all_ports;

CCGrafPort* CCGrafPort::as_port(void* ptr) {
  auto* port_ptr = reinterpret_cast<CCGrafPort*>(ptr);
  return all_ports.count(port_ptr) ? port_ptr : nullptr;
}

const CCGrafPort* CCGrafPort::as_port(const void* ptr) {
  const auto* port_ptr = reinterpret_cast<const CCGrafPort*>(ptr);
  return all_ports.count(port_ptr) ? port_ptr : nullptr;
}

CCGrafPort::CCGrafPort()
    : log(std::format("[CCGrafPort:{:016X}] ", reinterpret_cast<intptr_t>(this)), qd_log.min_level),
      is_window(false) {
  this->portBits = BitMap{};
  this->portRect = {0, 0, 0, 0};
  this->txFont = 0;
  this->txFace = 0;
  this->txMode = 0;
  this->txSize = 12;
  this->pnLoc = {0, 0};
  this->pnSize = {1, 1};
  this->pnMode = 0;
  this->portPixMap = nullptr;
  this->pnPixPat = nullptr;
  this->bkPixPat = nullptr;
  this->fgColor = 0x00000000;
  this->bgColor = 0xFFFFFFFF;
  this->rgbFgColor = {0x0000, 0x0000, 0x0000};
  this->rgbBgColor = {0xFFFF, 0xFFFF, 0xFFFF};
  all_ports.emplace(this);
  this->log.debug_f("Created");
}

CCGrafPort::CCGrafPort(const Rect& bounds, bool is_window) : CCGrafPort() {
  this->portRect = bounds;
  this->is_window = is_window;
  this->data.resize(this->portRect.right - this->portRect.left, this->portRect.bottom - this->portRect.top);
  this->log.debug_f("Resized to {}x{} with origin ({}, {}) and is_window={}",
      this->get_width(), this->get_height(), this->portRect.left, this->portRect.top, is_window ? "true" : "false");
  // We don't have to add this to all_ports here because the default
  // constructor already did that
}

CCGrafPort::~CCGrafPort() {
  this->log.debug_f("Destroyed");
  all_ports.erase(this);
}

void CCGrafPort::resize(size_t w, size_t h) {
  this->portRect.right = this->portRect.left + w;
  this->portRect.bottom = this->portRect.top + h;
  this->data.resize(w, h);
  this->log.debug_f("Resized to {}x{}", this->get_width(), this->get_height());
}

void CCGrafPort::erase_rect(const Rect& r) {
  if (this->bkPixPat) {
    this->draw_background_ppat(r);
  } else {
    uint32_t color = rgba8888_for_rgb_color(this->rgbBgColor);
    this->data.write_rect(r.left, r.top, r.right - r.left, r.bottom - r.top, color);
  }
}

void CCGrafPort::fill_rect(const Rect& r) {
  uint32_t color = rgba8888_for_rgb_color(this->rgbFgColor);
  this->data.write_rect(r.left, r.top, r.right - r.left, r.bottom - r.top, color);
}

void CCGrafPort::draw_rect_outline(const Rect& r) {
  uint32_t color = rgba8888_for_rgb_color(this->rgbFgColor);
  this->data.draw_horizontal_line(r.left, r.right - 1, r.top, 0, color);
  this->data.draw_horizontal_line(r.left, r.right - 1, r.bottom - 1, 0, color);
  this->data.draw_vertical_line(r.left, r.top, r.bottom - 1, 0, color);
  this->data.draw_vertical_line(r.right - 1, r.top, r.bottom - 1, 0, color);
}

void CCGrafPort::draw_ga11_data(const void* pixels, int sw, int sh, const Rect& rect) {
  // It's OK to const_cast pixels here because we only use the image as a source
  auto src = phosg::ImageGA11::from_data_reference(const_cast<void*>(pixels), sw, sh);
  ssize_t dw = rect.right - rect.left;
  ssize_t dh = rect.bottom - rect.top;
  this->data.copy_from_with_blend(src, rect.left, rect.top, dw, dh, 0, 0, sw, sh, phosg::ResizeMode::NEAREST_NEIGHBOR);
}

void CCGrafPort::draw_rgba8888_data(const void* pixels, int sw, int sh, const Rect& rect) {
  // It's OK to const_cast pixels here because we only use the image as a source
  auto src = phosg::ImageRGBA8888N::from_data_reference(const_cast<void*>(pixels), sw, sh);
  ssize_t dw = rect.right - rect.left;
  ssize_t dh = rect.bottom - rect.top;
  this->data.copy_from_with_blend(src, rect.left, rect.top, dw, dh, 0, 0, sw, sh, phosg::ResizeMode::NEAREST_NEIGHBOR);
}

void CCGrafPort::draw_decoded_pict_from_handle(PicHandle pict, const Rect& rect) {
  // See GetPicture for a description of what's going on here.
  auto r = read_from_handle(reinterpret_cast<Handle>(pict));
  const auto& header = r.get<DecodedPICTHeader>();
  if (header.version_opcode != 0x0011 || header.version_arg != 0x03FF || header.data_opcode != 0xFFFF) {
    throw std::runtime_error("Attempted to render a non-decoded PICT");
  }
  int w = header.bounds.right - header.bounds.left;
  int h = header.bounds.bottom - header.bounds.top;
  if (r.remaining() != phosg::ImageRGBA8888N::data_size(w, h)) {
    throw std::runtime_error(std::format("Decoded PICT data size is incorrect (expected 0x{:X}; received 0x{:X})", phosg::ImageRGBA8888N::data_size(w, h), r.remaining()));
  }

  this->draw_rgba8888_data(r.getv(r.remaining()), w, h, rect);
}

phosg::ImageRGBA8888N image_for_sdl_surface(SDL_Surface* surface) {
  if (surface->format != SDL_PIXELFORMAT_ARGB8888) {
    throw std::runtime_error(std::format("SDL surface must be 32-bit ARGB (instead, it is 0x{:08X})", static_cast<uint32_t>(surface->format)));
  }
  // TODO: Add support for row_bytes in phosg::Image so we can use
  // Image::from_data_reference here instead of copying the data
  phosg::ImageRGBA8888N ret(surface->w, surface->h);
  for (size_t y = 0; y < surface->h; y++) {
    const uint32_t* row_pixels = reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(surface->pixels) + (y * surface->pitch));
    for (size_t x = 0; x < surface->w; x++) {
      ret.write(x, y, phosg::rgba8888_for_argb8888(row_pixels[x]));
    }
  }
  return ret;
}

bool CCGrafPort::draw_text_ttf(TTF_Font* font, const std::string& processed_text, const Rect& rect) {
  size_t w = rect.right - rect.left;
  size_t h = rect.bottom - rect.top;
  auto sdl_color = sdl_color_for_rgb_color(this->rgbFgColor);
  auto text_surface = sdl_make_unique(TTF_RenderText_Blended_Wrapped(
      font, processed_text.data(), processed_text.size(), sdl_color, w + 50));
  if (!text_surface) {
    this->log.error_f("Failed to create surface when rendering text: {}", SDL_GetError());
    return false;
  } else {
    auto img = image_for_sdl_surface(text_surface.get());
    data.copy_from_with_blend(img, rect.left, rect.top, w, h, 0, 0);
    return true;
  }
}

bool CCGrafPort::draw_text_bitmap(const ResourceDASM::BitmapFontRenderer& renderer, const std::string& text, const Rect& rect) {
  uint32_t color32 = rgba8888_for_rgb_color(this->rgbFgColor);
  std::string wrapped_text = renderer.wrap_text_to_pixel_width(text, rect.right - rect.left);
  renderer.render_text(data, wrapped_text, rect.left, rect.top, rect.right, rect.bottom, color32);
  return true;
}

bool CCGrafPort::draw_text(const std::string& text, const Rect& r) {
  std::string processed_text = replace_param_text(text);
  if (processed_text.empty()) {
    return true;
  }

  auto font = load_font(this->txFont);
  bool success = false;

  if (std::holds_alternative<TTF_Font*>(font)) {
    this->log.debug_f("draw_text(\"{}\", {{x1={}, y1={}, x2={}, y2={}}}) font={} (TTF) size={} style={}",
        processed_text, r.left, r.top, r.right, r.bottom, this->txFont, this->txSize, this->txFace);
    auto tt_font = std::get<TTF_Font*>(font);
    TTF_SetFontSize(tt_font, this->txSize);
    set_font_style(tt_font, this->txFace);
    success = this->draw_text_ttf(tt_font, processed_text, r);

  } else if (std::holds_alternative<ResourceDASM::BitmapFontRenderer>(font)) {
    this->log.debug_f("draw_text(\"{}\", {{x1={}, y1={}, x2={}, y2={}}}) font={} (bitmap) size={} style={}",
        processed_text, r.left, r.top, r.right, r.bottom, this->txFont, this->txSize, this->txFace);
    auto bm_font = std::get<ResourceDASM::BitmapFontRenderer>(font);
    success = this->draw_text_bitmap(bm_font, processed_text, r);
  }

  if (!success) {
    this->log.error_f("No renderer is available for font {}; cannot render text \"{}\"", this->txFont, text);
  }
  return success;
}

static std::pair<size_t, size_t> pixel_dimensions_for_text(TTF_Font* tt_font, const std::string& text) {
  std::unique_ptr<TTF_Text, void (*)(TTF_Text*)> t(TTF_CreateText(NULL, tt_font, text.c_str(), 0), TTF_DestroyText);
  int w{0}, h{0};
  TTF_GetTextSize(t.get(), &w, &h);
  return std::make_pair(w, h);
}

void CCGrafPort::draw_text(const std::string& text) {
  std::string processed_text = replace_param_text(text);

  auto font = load_font(this->txFont);
  int width = -1;
  if (std::holds_alternative<TTF_Font*>(font)) {
    auto tt_font = std::get<TTF_Font*>(font);
    TTF_SetFontSize(tt_font, this->txSize);
    set_font_style(tt_font, this->txFace);

    // The pen location, passed in as the x and y parameters, is at the baseline of the text, to
    // the left. So, we need to account for this in our display rect.
    // Descent is a negative number, representing the pixels below the baseline the text may extend.
    auto descent = TTF_GetFontDescent(tt_font);
    this->log.debug_f("draw_text(\"{}\") font={} (TTF) size={} style={} descent={}",
        processed_text, this->txFont, this->txSize, this->txFace, descent);

    auto [w, h] = pixel_dimensions_for_text(tt_font, processed_text);
    Rect r{
        static_cast<int16_t>(this->pnLoc.v - h - descent),
        this->pnLoc.h,
        static_cast<int16_t>(this->pnLoc.v - descent),
        static_cast<int16_t>(this->pnLoc.h + w)};
    width = this->draw_text_ttf(tt_font, processed_text, r) ? w : -1;

  } else if (std::holds_alternative<ResourceDASM::BitmapFontRenderer>(font)) {

    // Like the TTF case, we need to account for the text being anchored at the baseline instead
    // of the upper-left corner. The renderer doesn't overwrite any pixels except those that are
    // part of each glyph, so we can render directly to data here.
    auto& bm_font = std::get<ResourceDASM::BitmapFontRenderer>(font);
    auto font = bm_font.get_font();
    ssize_t descent = font->max_ascent;
    this->log.debug_f("draw_text(\"{}\") font={} (bitmap) size={} style={} descent={}",
        processed_text, this->txFont, this->txSize, this->txFace, descent);
    auto [text_width, text_height] = bm_font.pixel_dimensions_for_text(processed_text);
    bm_font.render_text(
        this->data,
        text,
        this->pnLoc.h,
        this->pnLoc.v - descent,
        this->pnLoc.h + text_width,
        this->pnLoc.v + text_height - descent,
        rgba8888_for_rgb_color(this->rgbFgColor));
    width = text_width;
  }

  if (width == -1) {
    this->log.error_f("No renderer is available for font ID {}; cannot render text \"{}\"", this->txFont, text);
  } else {
    this->pnLoc.h += width;
  }
}

int CCGrafPort::measure_text(const std::string& text) {
  std::string processed_text = replace_param_text(text);

  auto font = load_font(this->txFont);
  int width = -1;
  if (std::holds_alternative<TTF_Font*>(font)) {
    auto tt_font = std::get<TTF_Font*>(font);
    TTF_SetFontSize(tt_font, this->txSize);
    set_font_style(tt_font, this->txFace);
    return pixel_dimensions_for_text(tt_font, processed_text).first;
  } else if (std::holds_alternative<ResourceDASM::BitmapFontRenderer>(font)) {
    auto& bm_font = std::get<ResourceDASM::BitmapFontRenderer>(font);
    return bm_font.pixel_dimensions_for_text(processed_text).first;
  } else {
    this->log.error_f("No renderer is available for font ID {}; cannot measure text \"{}\"", this->txFont, text);
    return -1;
  }
}

void CCGrafPort::draw_rect(const Rect& r) {
  this->data.write_rect(r.left, r.top, r.right - r.left, r.bottom - r.top, rgba8888_for_rgb_color(this->rgbFgColor));
}

// Derived from https://en.wikipedia.org/wiki/Ellipse#In_Cartesian_coordinates and
// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
template <typename FnT>
  requires(std::is_invocable_r_v<void, FnT, size_t, size_t>)
void draw_oval_custom(const Rect& r, FnT&& write) {
  // Compute semi-major and semi-minor axes, center coordinates, and focus
  // distance from center
  auto a = (r.right - r.left) / 2.0;
  auto b = (r.bottom - r.top) / 2.0;
  int x0{}, y0{}, x{};
  int y = 0;
  bool vertical{false};

  if (a < b) {
    std::swap(a, b);
    x0 = r.right - b;
    y0 = r.bottom - a;
    x = b;
    vertical = true;
  } else {
    x0 = r.right - a;
    y0 = r.bottom - b;
    x = a;
  }

  int c = sqrt(a * a - b * b);

  // Calculate ellipse pixels in coordinates that are relative to the center,
  // then translate to actual center when drawing. Start at (x, 0), first quadrant

  // Foci
  // if (vertical) {
  //   this->data.write(x0, y0 + c, color);
  //   this->data.write(x0, y0 - c, color);
  // } else {
  //   this->data.write(x0 + c, y0, color);
  //   this->data.write(x0 - c, y0, color);
  // }

  int dx_f1{}, dx_f2{}, dy_f1{}, dy_f2{};
  while (x > 0) {
    // Mirror the pixel to quadrants 2, 3, and 4
    write(x0 + x, y0 + y);
    write(x0 + x, y0 - y);
    write(x0 - x, y0 + y);
    write(x0 - x, y0 - y);

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
    write(x0, y0 + a);
    write(x0, y0 - a);
  } else {
    write(x0, y0 + b);
    write(x0, y0 - b);
  }
}

static phosg::ImageRGB888 reference_image_for_ppat(PixPatHandle ppat) {
  PixMapHandle pmap = (*ppat)->patMap;
  Rect bounds = (*pmap)->bounds;
  int w = bounds.right - bounds.left;
  int h = bounds.bottom - bounds.top;
  return phosg::ImageRGB888::from_data_reference(*(*ppat)->patData, w, h);
}

void CCGrafPort::draw_oval(const Rect& r) {
  // TODO: We should respect the pen size here
  switch (this->pnMode) {
    case 0x00: { // srcCopy
      uint32_t color = rgba8888_for_rgb_color(this->rgbFgColor);
      draw_oval_custom(r, [&](size_t x, size_t y) -> void {
        this->data.write(x, y, color);
      });
      break;
    }
    case 0x02: // srcXor
      draw_oval_custom(r, [&](size_t x, size_t y) -> void {
        this->data.write(x, y, phosg::invert(this->data.read(x, y)));
      });
      break;
    case 0x08: { // patCopy
      if (!this->pnPixPat) {
        throw std::logic_error("Cannot draw with patCopy mode unless PenPixPat was previously set");
      }
      const auto ppat = reference_image_for_ppat(this->pnPixPat);
      draw_oval_custom(r, [&](size_t x, size_t y) -> void {
        this->data.write(x, y, ppat.read(x % ppat.get_width(), y % ppat.get_height()));
      });
      break;
    }
    default:
      throw std::runtime_error("Unimplemented draw_oval transfer mode");
  }
}

void CCGrafPort::draw_line(const Point& start, const Point& end) {
  // TODO: We should respect the pen size here
  switch (this->pnMode) {
    case 0x00: // srcCopy
      this->data.draw_line(start.h, start.v, end.h, end.v, rgba8888_for_rgb_color(this->rgbFgColor));
      break;
    case 0x02: // srcXor
      this->data.draw_line_custom(start.h, start.v, end.h, end.v, [this](size_t x, size_t y) -> void {
        this->data.write(x, y, phosg::invert(this->data.read(x, y)));
      });
      break;
    case 0x08: { // patCopy
      if (!this->pnPixPat) {
        throw std::logic_error("Cannot draw with patCopy mode unless PenPixPat was previously set");
      }
      const auto ppat = reference_image_for_ppat(this->pnPixPat);
      this->data.draw_line_custom(start.h, start.v, end.h, end.v, [this, &ppat](size_t x, size_t y) -> void {
        this->data.write(x, y, ppat.read(x % ppat.get_width(), y % ppat.get_height()));
      });
      break;
    }
    default:
      throw std::runtime_error("Unimplemented draw_line transfer mode");
  }
}

void CCGrafPort::draw_line_to(const Point& end) {
  this->draw_line(this->pnLoc, end);
  this->pnLoc = end;
}

void CCGrafPort::draw_background_ppat() {
  auto ppat = reference_image_for_ppat(this->bkPixPat);
  for (size_t y = 0; y < this->data.get_height(); y += ppat.get_height()) {
    for (size_t x = 0; x < this->data.get_width(); x += ppat.get_width()) {
      this->data.copy_from(ppat, x, y, ppat.get_width(), ppat.get_height(), 0, 0);
    }
  }
}

void CCGrafPort::draw_background_ppat(const Rect& rect) {
  PixMapHandle pmap = (*this->bkPixPat)->patMap;
  Rect bounds = (*pmap)->bounds;
  int w = bounds.right - bounds.left;
  int h = bounds.bottom - bounds.top;
  auto pattern = phosg::ImageRGB888::from_data_reference(*(*this->bkPixPat)->patData, w, h);

  ssize_t rx = rect.left, ry = rect.top, rw = rect.right - rect.left, rh = rect.bottom - rect.top;
  this->data.clamp_rect(rx, ry, rw, rh);
  for (ssize_t y = ry; y < ry + rh; y++) {
    for (ssize_t x = rx; x < rx + rw; x++) {
      this->data.write(x, y, pattern.read(x % pattern.get_width(), y % pattern.get_height()));
    }
  }
}

void CCGrafPort::copy_from(const CCGrafPort& src, const Rect& src_rect, const Rect& dst_rect, int16_t mode) {
  int src_w = src_rect.right - src_rect.left;
  int src_h = src_rect.bottom - src_rect.top;
  int dst_w = dst_rect.right - dst_rect.left;
  int dst_h = dst_rect.bottom - dst_rect.top;

  // TODO: Implement the rest of these if they become necessary. See Inside
  // Macintosh: QuickDraw, 3-115
  switch (mode) {
    case 0x00: // srcCopy
      this->data.copy_from(src.data, dst_rect.left, dst_rect.top, dst_w, dst_h, src_rect.left, src_rect.top, src_w, src_h, phosg::ResizeMode::NEAREST_NEIGHBOR);
      break;

    case 0x01: { // srcOr
      uint32_t fg_color = rgba8888_for_rgb_color(this->rgbFgColor);
      this->data.copy_from_with_custom(
          src.data, dst_rect.left, dst_rect.top, dst_w, dst_h, src_rect.left, src_rect.top, src_w, src_h, phosg::ResizeMode::NEAREST_NEIGHBOR,
          [fg_color](uint32_t dst_c, uint32_t src_c) -> uint32_t {
            if (src_c == 0x000000FF) {
              return fg_color;
            } else if (src_c == 0xFFFFFFFF) {
              return dst_c;
            } else {
              // Inside Macintosh: QuickDraw, page 4-33, says "Apply weighted portions of foreground color" if the
              // source pixel isn't white or black. We take this to mean that the destination pixel should be linearly
              // interpolated (in each channel) between its existing color and the foreground color, based on the
              // value in each channel of the source pixel.
              uint8_t sr = phosg::get_r(src_c);
              uint8_t sg = phosg::get_g(src_c);
              uint8_t sb = phosg::get_b(src_c);
              return phosg::rgba8888(
                  (sr * phosg::get_r(dst_c) + (0xFF - sr) * phosg::get_r(fg_color)) / 0xFF,
                  (sg * phosg::get_g(dst_c) + (0xFF - sg) * phosg::get_g(fg_color)) / 0xFF,
                  (sb * phosg::get_b(dst_c) + (0xFF - sb) * phosg::get_b(fg_color)) / 0xFF,
                  0xFF);
            }
          });
      break;
    }

    case 0x24: // transparent
      this->data.copy_from_with_source_color_mask(
          src.data, dst_rect.left, dst_rect.top, dst_w, dst_h, src_rect.left, src_rect.top, src_w, src_h,
          rgba8888_for_rgb_color(this->rgbBgColor), phosg::ResizeMode::NEAREST_NEIGHBOR);
      break;

    case 0x02: // srcXor
    case 0x03: // srcBic
    case 0x04: // notSrcCopy
    case 0x05: // notSrcOr
    case 0x06: // notSrcXor
    case 0x07: // notSrcBic
    case 0x20: // blend
    case 0x21: // addPin
    case 0x22: // addOver
    case 0x23: // subPin
    case 0x25: // addMax
    case 0x26: // subOver
    case 0x27: // adMin
      throw std::runtime_error("Unimplemented CopyBits transfer mode");
    default:
      throw std::runtime_error("Unknown CopyBits transfer mode");
  }
}

///////////////////////////////////////////////////////////////////////////////

// Originally declared in variables.h. It seems that `qd` was introduced by Myriad during the
// port to PC in place of Classic Mac's global QuickDraw context. We can repurpose it here
// for easier access in our code, while still exposing a C-compatible struct.
QuickDrawGlobals qd;

CCGrafPort& get_default_port() {
  static std::unique_ptr<CCGrafPort> default_port;
  if (!default_port) {
    default_port = std::make_unique<CCGrafPort>();
  }
  return *default_port.get();
}

Rect rect_from_reader(phosg::StringReader& data) {
  Rect r;
  r.top = data.get_u16b();
  r.left = data.get_u16b();
  r.bottom = data.get_u16b();
  r.right = data.get_u16b();
  return r;
}

Boolean PtInRect(Point pt, const Rect* r) {
  return (pt.v >= r->top) && (pt.h >= r->left) && (pt.v < r->bottom) && (pt.h < r->right);
}

void SetRect(Rect* r, int16_t left, int16_t top, int16_t right, int16_t bottom) {
  r->left = left;
  r->top = top,
  r->right = right;
  r->bottom = bottom;
}

void SetPt(Point* pt, int16_t h, int16_t v) {
  pt->h = h;
  pt->v = v;
}

RGBColor color_const_to_rgb(int32_t color_const) {
  switch (color_const) {
    case whiteColor:
      return RGBColor{65535, 65535, 65535};
    case blackColor:
      return RGBColor{0, 0, 0};
    case yellowColor:
      return RGBColor{65535, 65535, 0};
    case redColor:
      return RGBColor{65535, 0, 0};
    case cyanColor:
      return RGBColor{0, 65535, 65535};
    case greenColor:
      return RGBColor{0, 65535, 0};
    case blueColor:
      return RGBColor{0, 0, 65535};
    default:
      qd_log.error_f("Unrecognized color constant {}", color_const);
      break;
  }
  return RGBColor{};
}

PixPatHandle GetPixPat(uint16_t patID) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_ppat, patID);
  if (!data_handle) {
    throw std::runtime_error(std::format("Resource ppat:{} was not found", patID));
  }
  auto r = read_from_handle(data_handle);
  const auto& header = r.get<ResourceDASM::PixelPatternResourceHeader>();

  const auto& pixmap_header = r.pget<ResourceDASM::PixelMapHeader>(header.pixel_map_offset + 4);
  auto patMap = NewHandleTyped<PixMap>();
  (*patMap)->pixelSize = pixmap_header.pixel_size;
  (*patMap)->bounds.top = pixmap_header.bounds.y1;
  (*patMap)->bounds.left = pixmap_header.bounds.x1;
  (*patMap)->bounds.bottom = pixmap_header.bounds.y2;
  (*patMap)->bounds.right = pixmap_header.bounds.x2;

  ResourceDASM::ResourceFile::DecodedPattern pattern = ResourceDASM::ResourceFile::decode_ppat(
      *data_handle, GetHandleSize(data_handle));

  auto ret_handle = NewHandleTyped<PixPat>();
  auto& ret = **ret_handle;
  ret.patType = header.type;
  ret.patMap = patMap;
  ret.patData = NewHandleWithData(pattern.pattern.get_data(), pattern.pattern.get_data_size());
  ret.patXData = nullptr;
  ret.patXValid = 0;
  ret.patXMap = 0;
  ret.pat1Data.pat[0] = pattern.raw_monochrome_pattern >> 56;
  ret.pat1Data.pat[1] = pattern.raw_monochrome_pattern >> 48;
  ret.pat1Data.pat[2] = pattern.raw_monochrome_pattern >> 40;
  ret.pat1Data.pat[3] = pattern.raw_monochrome_pattern >> 32;
  ret.pat1Data.pat[4] = pattern.raw_monochrome_pattern >> 24;
  ret.pat1Data.pat[5] = pattern.raw_monochrome_pattern >> 16;
  ret.pat1Data.pat[6] = pattern.raw_monochrome_pattern >> 8;
  ret.pat1Data.pat[7] = pattern.raw_monochrome_pattern;
  return ret_handle;
}

void DisposePixPat(PixPatHandle ppat) {
  if ((*ppat)->patData) {
    DisposeHandle((*ppat)->patData);
  }
  DisposeHandleTyped(ppat);
}

PicHandle GetPicture(int16_t id) {
  // The GetPicture Mac Classic syscall must return a Handle to a decoded Picture resource,
  // but it must also be the same Handle we use to index loaded Resources in the ResourceManager.
  // Otherwise, subsequent calls to DetachResource or ReleaseResource would fail to find it.
  //
  // By default, the GetResource call leaves the raw bytes of the resource in data_handle. To
  // satisfy the above, we replace that with the fully decoded Picture resource. To indicate that
  // the handle contains decoded data, we prepend a header claiming that it's PICT version 3
  // (there were only PICT versions 1 and 2 used in QuickDraw).
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_PICT, id);
  if (!data_handle) {
    return nullptr;
  }

  {
    auto r = read_from_handle(data_handle);
    const auto& header = r.get<DecodedPICTHeader>();
    if (header.version_opcode == 0x0011 && header.version_arg == 0x03FF && header.data_opcode == 0xFFFF) {
      return reinterpret_cast<PicHandle>(data_handle);
    }
  }

  auto p = ResourceDASM::ResourceFile::decode_PICT_only(*data_handle, GetHandleSize(data_handle));
  if (p.image.get_height() == 0 || p.image.get_width() == 0) {
    throw std::runtime_error(std::format("Failed to decode PICT {}", id));
  }

  // Generate the decoded PICT data stream
  DecodedPICTHeader header;
  header.size = 0; // This is common for Picture objects; it's ignored by QD
  header.bounds.left = 0;
  header.bounds.top = 0;
  header.bounds.right = p.image.get_width();
  header.bounds.bottom = p.image.get_height();
  header.version_opcode = 0x0011;
  header.version_arg = 0x03FF;
  header.data_opcode = 0xFFFF;

  phosg::StringWriter w;
  w.put<DecodedPICTHeader>(header);
  w.write(p.image.get_data(), p.image.get_data_size());

  // Now, free the original data handle buffer with the raw bytes, and change the data_handle
  // to contain the new pointer to the decoded image.
  replace_handle_data(data_handle, w.str().data(), w.str().size());

  return reinterpret_cast<PicHandle>(data_handle);
}

void ForeColor(int32_t color) {
  qd.thePort->fgColor = color;
  qd.thePort->rgbFgColor = color_const_to_rgb(color);
}

void BackColor(int32_t color) {
  qd.thePort->bgColor = color;
  qd.thePort->rgbBgColor = color_const_to_rgb(color);
}

void GetBackColor(RGBColor* color) {
  *color = qd.thePort->rgbBgColor;
}

void GetForeColor(RGBColor* color) {
  *color = qd.thePort->rgbFgColor;
}

void SetPort(CGrafPtr port) {
  qd.thePort = port;
}

// Called in main.c, this function passes in the location of the global
// QuickDraw context for initialization. However, since we've taken over
// the implementation of the global qd object and have statically allocated
// its members, there is no need for further initialization beyond updating
// qd.thePort to point at the default port
void InitGraf(QuickDrawGlobals*) {
  qd.thePort = &get_default_port();
}

CCGrafPort& current_port() {
  auto* ret = CCGrafPort::as_port(qd.thePort);
  if (!ret) {
    throw std::logic_error("current port is not a CCGrafPort");
  }
  return *ret;
}

void TextFont(uint16_t font) {
  qd.thePort->txFont = font;
}

void TextMode(int16_t mode) {
  qd.thePort->txMode = mode;
}

void TextSize(uint16_t size) {
  qd.thePort->txSize = size;
}

void TextFace(int16_t face) {
  qd.thePort->txFace = face;
}

void GetPort(GrafPtr* port) {
  *port = reinterpret_cast<GrafPtr>(qd.thePort);
}

void RGBBackColor(const RGBColor* color) {
  qd.thePort->rgbBgColor = *color;
}

void RGBForeColor(const RGBColor* color) {
  qd.thePort->rgbFgColor = *color;
}

CIconHandle GetCIcon(uint16_t iconID) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_cicn, iconID);
  auto decoded_cicn = ResourceDASM::ResourceFile::decode_cicn(*data_handle, GetHandleSize(data_handle));

  CIconHandle h = NewHandleTyped<CIcon>();
  (*h)->iconData = NewHandleWithData(decoded_cicn.image.get_data(), decoded_cicn.image.get_data_size());
  (*h)->bitmapData = NewHandleWithData(decoded_cicn.bitmap.get_data(), decoded_cicn.bitmap.get_data_size());
  (*h)->iconPMap.bounds = Rect{
      0,
      0,
      static_cast<int16_t>(decoded_cicn.image.get_height()),
      static_cast<int16_t>(decoded_cicn.image.get_width())};
  (*h)->iconPMap.pixelSize = 32;
  (*h)->iconBMap.bounds = (*h)->iconPMap.bounds;
  return h;
}

OSErr DisposeCIcon(CIconHandle icon) {
  if ((*icon)->iconData) {
    DisposeHandle((*icon)->iconData);
  }
  DisposeHandleTyped(icon);
  return noErr;
}

OSErr PlotCIcon(const Rect* r, CIconHandle icon) {
  auto bounds = (*icon)->iconPMap.bounds;
  int w = bounds.right - bounds.left;
  int h = bounds.bottom - bounds.top;
  auto& port = current_port();
  port.log.debug_f("PlotCIcon({{x0={}, y0={}, x1={}, y1={}}}, {:p})", r->left, r->top, r->right, r->bottom, static_cast<void*>(icon));
  port.draw_rgba8888_data(*((*icon)->iconData), w, h, *r);
  WindowManager::instance().recomposite_from_window(port);
  return noErr;
}

OSErr PlotCIconBitmap(const Rect* r, CIconHandle icon) {
  auto bounds = (*icon)->iconBMap.bounds;
  int w = bounds.right - bounds.left;
  int h = bounds.bottom - bounds.top;
  auto& port = current_port();
  port.log.debug_f("PlotCIconBitmap({{x0={}, y0={}, x1={}, y1={}}}, {:p})", r->left, r->top, r->right, r->bottom, static_cast<void*>(icon));
  port.draw_ga11_data(*((*icon)->bitmapData), w, h, *r);
  WindowManager::instance().recomposite_from_window(port);
  return noErr;
}

void BackPixPat(PixPatHandle ppat) {
  qd.thePort->bkPixPat = ppat;
}

void MoveTo(int16_t h, int16_t v) {
  qd.thePort->pnLoc = Point{v, h};
}

void InsetRect(Rect* r, int16_t dh, int16_t dv) {
  r->left += dh;
  r->right -= dh;
  r->top += dv;
  r->bottom -= dv;
}

void PenPixPat(PixPatHandle ppat) {
  qd.thePort->pnPixPat = ppat;
  qd.thePort->pnMode = 8; // patCopy
}

void PenSize(int16_t width, int16_t height) {
  qd.thePort->pnSize = {height, width};
}

void PenMode(int16_t mode) {
  qd.thePort->pnMode = mode;
}

// The gdh return parameter, a graphics device handle, is only stored temporarily
// by Realmz while it swaps out the current GWorld, then is used to reset to the
// original graphics device. Since we don't actually need to use the graphics device,
// we can ignore it.
void GetGWorld(CGrafPtr* port, GDHandle* gdh) {
  GetPort(port);
}

void SetGWorld(CGrafPtr port, GDHandle gdh) {
  SetPort(port);
}

PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld) {
  return NULL;
}

QDErr NewGWorld(GWorldPtr* offscreenGWorld, int16_t pixelDepth, const Rect* boundsRect, CTabHandle cTable,
    GDHandle aGDevice, GWorldFlags flags) {
  *offscreenGWorld = new CCGrafPort(*boundsRect, false);
  return 0;
}

void DisposeGWorld(GWorldPtr offscreenWorld) {
  delete reinterpret_cast<CCGrafPort*>(offscreenWorld);
}

void DrawString(ConstStr255Param s) {
  auto str = string_for_pstr<255>(s);
  auto& port = current_port();
  port.log.debug_f("DrawString(\"{}\") @ pnLoc={{x={}, y={}}}", str, port.pnLoc.h, port.pnLoc.v);
  port.draw_text(str);
  WindowManager::instance().recomposite_from_window(port);
}

int16_t TextWidth(const void* textBuf, int16_t firstByte, int16_t byteCount) {
  // Realmz always calls this procedure with 0 as the first byte, and the full
  // strlen as the byteCount, so we can ignore those parameters and just measure
  // the full string.
  // Realmz also seems to only call this with cstrings, so we're good there as well.
  return current_port().measure_text(static_cast<const char*>(textBuf));
}

void DrawPicture(PicHandle pict, const Rect* r) {
  auto& port = current_port();
  port.log.debug_f("DrawPicture({:p}, {{x0={}, y0={}, x1={}, y1={}}})", static_cast<void*>(pict), r->left, r->top, r->right, r->bottom);
  port.draw_decoded_pict_from_handle(pict, *r);
  WindowManager::instance().recomposite_from_window(port);
}

void LineTo(int16_t h, int16_t v) {
  auto& port = current_port();
  port.log.debug_f("LineTo({}, {})", h, v);
  port.draw_line_to(Point{.v = v, .h = h});
  WindowManager::instance().recomposite_from_window(port);
}

void FrameOval(const Rect* r) {
  auto& port = current_port();
  port.log.debug_f("FrameOval({{x0={}, y0={}, x1={}, y1={}}}) mode={:04X} fg={:08X}",
      r->left, r->top, r->right, r->bottom, port.pnMode, rgba8888_for_rgb_color(port.rgbFgColor));
  port.draw_oval(*r);
  WindowManager::instance().recomposite_from_window(port);
}

void CopyBits(const BitMap* src, BitMap* dst, const Rect* src_r, const Rect* dst_r, int16_t mode, RgnHandle maskRgn) {
  auto* src_port = CCGrafPort::as_port(src);
  auto* dst_port = CCGrafPort::as_port(dst);
  if (!src_port) {
    throw std::runtime_error("CopyBits called with a src that isn't a CCGrafPort");
  }
  if (!dst_port) {
    throw std::runtime_error("CopyBits called with a dst that isn't a CCGrafPort");
  }

  dst_port->log.debug_f("CopyBits({}, {}, {{x0={}, y0={}, x1={}, y1={}}}, {{x0={}, y0={}, x1={}, y1={}}}, {:04X}, {:p})", src_port->ref(), dst_port->ref(), src_r->left, src_r->top, src_r->right, src_r->bottom, dst_r->left, dst_r->top, dst_r->right, dst_r->bottom, mode, static_cast<void*>(maskRgn));
  dst_port->copy_from(*src_port, *src_r, *dst_r, mode);
  WindowManager::instance().recomposite_from_window(*dst_port);
}

void CopyMask(const BitMap* src, const BitMap* mask, BitMap* dst, const Rect* src_r, const Rect* mask_r, const Rect* dst_r) {

  auto* src_port = CCGrafPort::as_port(src);
  auto* mask_port = CCGrafPort::as_port(mask);
  auto* dst_port = CCGrafPort::as_port(dst);
  if (!src_port) {
    throw std::runtime_error("CopyMask called with a src that isn't a CCGrafPort");
  }
  if (!mask_port) {
    throw std::runtime_error("CopyMask called with a mask that isn't a CCGrafPort");
  }
  if (!dst_port) {
    throw std::runtime_error("CopyMask called with a dst that isn't a CCGrafPort");
  }

  dst_port->log.debug_f("CopyMask({}, {}, {}, {{x0={}, y0={}, x1={}, y1={}}}, {{x0={}, y0={}, x1={}, y1={}}}, {{x0={}, y0={}, x1={}, y1={}}})",
      src_port->ref(), mask_port->ref(), dst_port->ref(),
      src_r->left, src_r->top, src_r->right, src_r->bottom,
      mask_r->left, mask_r->top, mask_r->right, mask_r->bottom,
      dst_r->left, dst_r->top, dst_r->right, dst_r->bottom);

  // According to IM: QuickDraw 3-119, mask_r must be the same size as src_r, but Realmz violates this condition.
  // Empirically it seems that the right thing to do is to ignore the size of mask_r and just use its origin, so we do
  // that here. src_r and dst_r still must be the same size though (this is not a requirement in IM: QuickDraw, but
  // Realmz always calls this with the same src_r and dst_r, and we choose to be lazy).
  ssize_t src_w = src_r->right - src_r->left;
  ssize_t src_h = src_r->bottom - src_r->top;
  ssize_t dst_w = dst_r->right - dst_r->left;
  ssize_t dst_h = dst_r->bottom - dst_r->top;
  if (src_w != dst_w || src_h != dst_h) {
    throw std::runtime_error(std::format("CopyMask dest size ({}x{}) does not match source size ({}x{})", dst_w, dst_h, src_w, src_h));
  }

  for (size_t y = 0; y < dst_h; y++) {
    for (size_t x = 0; x < dst_w; x++) {
      if (mask_port->data.check(mask_r->left + x, mask_r->top + y) &&
          mask_port->data.read(mask_r->left + x, mask_r->top + y) == 0x000000FF) {
        dst_port->data.write(dst_r->left + x, dst_r->top + y, src_port->data.read(src_r->left + x, src_r->top + y));
      }
    }
  }
}

void ScrollRect(const Rect* r, int16_t dh, int16_t dv, RgnHandle updateRgn) {
  // Note: Realmz only calls ScrollRect with updateRgn = nullptr, so we ignore
  // it.

  Rect src_rect = *r;
  Rect dst_rect = *r;
  if (dh > 0) {
    src_rect.right -= dh;
    dst_rect.left += dh;
  } else {
    src_rect.left -= dh;
    dst_rect.right += dh;
  }
  if (dv > 0) {
    src_rect.bottom -= dv;
    dst_rect.top += dv;
  } else {
    src_rect.top -= dv;
    dst_rect.bottom += dv;
  }

  auto port = CCGrafPort::as_port(qd.thePort);
  if (!port) {
    throw std::logic_error("qd.thePort is not a CCGrafPort");
  }

  ssize_t w = dst_rect.right - dst_rect.left;
  ssize_t h = dst_rect.bottom - dst_rect.top;
  if (w != (src_rect.right - src_rect.left) || h != (src_rect.bottom - src_rect.top)) {
    throw std::logic_error("src_rect and dst_rect are not the same size in ScrollRect");
  } else if (w > 0 && h > 0) {
    // If the content is being moved down, we need to iterate in the opposite
    // order so we don't overwrite content that hasn't been moved yet (and then
    // duplicate it when we read it). Technically we should handle this along
    // the horizontal dimension too, but Realmz only calls ScrollRect with dh=0
    // and therefore I'm lazy.
    if (dv > 0) {
      for (ssize_t y = h - 1; y >= 0; y--) {
        for (ssize_t x = 0; x < w; x++) {
          port->data.write(dst_rect.left + x, dst_rect.top + y, port->data.read(src_rect.left + x, src_rect.top + y));
        }
      }
    } else {
      port->copy_from(*port, src_rect, dst_rect, 0);
    }
  }
}

void EraseRect(const Rect* r) {
  auto& port = current_port();
  port.log.debug_f("EraseRect({{x0={}, y0={}, x1={}, y1={}}})", r->left, r->top, r->right, r->bottom);
  port.erase_rect(*r);
  WindowManager::instance().recomposite_from_window(port);
}

void PaintRect(const Rect* r) {
  auto& port = current_port();
  port.log.debug_f("PaintRect({{x0={}, y0={}, x1={}, y1={}}})", r->left, r->top, r->right, r->bottom);
  port.fill_rect(*r);
  WindowManager::instance().recomposite_from_window(port);
}

void FrameRect(const Rect* r) {
  auto& port = current_port();
  port.log.debug_f("FrameRect({{x0={}, y0={}, x1={}, y1={}}})", r->left, r->top, r->right, r->bottom);
  port.draw_rect_outline(*r);
  WindowManager::instance().recomposite_from_window(port);
}

Boolean SectRect(const Rect* a, const Rect* b, Rect* dest) {
  Rect res = {
      std::max<int16_t>(a->top, b->top),
      std::max<int16_t>(a->left, b->left),
      std::min<int16_t>(a->bottom, b->bottom),
      std::min<int16_t>(a->right, b->right),
  };
  if (res.top >= res.bottom || res.left >= res.right) {
    dest->top = 0;
    dest->left = 0;
    dest->bottom = 0;
    dest->right = 0;
    return false;
  } else {
    *dest = res;
    return true;
  }
}

int32_t DeltaPoint(Point a, Point b) {
  // Inside Macintosh I-475:
  // DeltaPoint subtracts the coordinates of b from the coordinates of a. The
  // high-order word of the result is the difference of the vertical
  // coordinates and the low-order word is the difference of the horizontal
  // coordinates.
  int32_t dv = a.v - b.v;
  int32_t dh = a.h - b.h;
  return ((dv << 16) & 0xFFFF0000) | (dh & 0x0000FFFF);
}

void GetPortBounds(CGrafPtr port, Rect* rect) {
  auto* cc_port = CCGrafPort::as_port(port);
  if (!cc_port) {
    throw std::runtime_error("GetPortBounds called with a port that isn't a CCGrafPort");
  }
  *rect = cc_port->portRect;
}

void ErasePortRect() {
  /* TODO(fuzziqersoftware): It seems that disabling this function makes things
   * work better; for example, the simple encounter dialog box is blank unless
   * this function does nothing. We should figure out why this is the case;
   * presumably this function was originally written for a reason and isn't
   * supposed to do nothing.

  auto* cc_port = CCGrafPort::as_port(qd.thePort);
  if (!cc_port) {
    throw std::runtime_error("GetPortBounds called with a port that isn't a CCGrafPort");
  }
  // EraseRect expects a rect in port-space, not global-space
  Rect r = {
      .left = 0,
      .top = 0,
      .right = static_cast<int16_t>(cc_port->portRect.right - cc_port->portRect.left),
      .bottom = static_cast<int16_t>(cc_port->portRect.bottom - cc_port->portRect.top),
  };
  EraseRect(&r);
  */
}

// Cursor functions

struct ColorCursor {
  // Opaque (to the caller) structure representing a loaded cursor.
  bool is_orphaned = false;
  int16_t resource_id;
  std::shared_ptr<ResourceDASM::ResourceFile::DecodedColorCursorResource> decoded;
  sdl_surface_ptr sdl_surface;
  sdl_cursor_ptr sdl_cursor;
};

static ColorCursor** current_cursor_handle = nullptr;
static ssize_t cursor_hide_level = 0;

CCrsrHandle GetCCursor(uint16_t resource_id) {
  ColorCursor cursor;
  cursor.resource_id = resource_id;

  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_crsr, resource_id);
  cursor.decoded = std::make_shared<ResourceDASM::ResourceFile::DecodedColorCursorResource>(
      ResourceDASM::ResourceFile::decode_crsr(*data_handle, GetHandleSize(data_handle)));

  cursor.sdl_surface = sdl_make_unique(SDL_CreateSurfaceFrom(
      cursor.decoded->image.get_width(),
      cursor.decoded->image.get_height(),
      SDL_PIXELFORMAT_RGBA8888,
      const_cast<uint32_t*>(cursor.decoded->image.get_data()),
      4 * cursor.decoded->image.get_width()));

  cursor.sdl_cursor = sdl_make_unique(SDL_CreateColorCursor(
      cursor.sdl_surface.get(), cursor.decoded->hotspot_x, cursor.decoded->hotspot_y));

  ColorCursor** ret = NewHandleTyped<ColorCursor>(std::move(cursor));
  return reinterpret_cast<Handle>(ret);
}

void DisposeCCursor(CCrsrHandle handle) {
  ColorCursor** cursor = reinterpret_cast<ColorCursor**>(handle);
  if (current_cursor_handle == cursor) {
    // Can't free this cursor yet because it's in use, but Realmz thinks it's
    // freed, so mark it for later freeing when the cursor is next changed
    (*cursor)->is_orphaned = true;
  } else {
    DisposeHandleTyped(cursor);
  }
}

void SetCCursor(CCrsrHandle handle) {
  ColorCursor** cursor = reinterpret_cast<ColorCursor**>(handle);
  if (current_cursor_handle == cursor) {
    return;
  }
  if (current_cursor_handle && (*current_cursor_handle)->is_orphaned) {
    DisposeHandleTyped(current_cursor_handle);
  }
  current_cursor_handle = cursor;

  SDL_SetCursor((*cursor)->sdl_cursor.get());
}

void ObscureCursor(void) {
  // TODO: It doesn't appear that SDL has an easy equivalent of this; we would
  // probably have to call SDL_HideCursor, save some kind of flag on the
  // current cursor, then call SDL_ShowCursor again when we receive any mouse
  // movement event. But QuickDraw doesn't know about the Event Manager (and it
  // should stay that way if possible), so we don't implement this currently.
  // The lack of this implementation probably won't affect the gameplay
  // experience much.
}

void HideCursor(void) {
  if (cursor_hide_level == 0) {
    SDL_HideCursor();
  }
  cursor_hide_level++;
}

void ShowCursor(void) {
  cursor_hide_level--;
  if (cursor_hide_level == 0) {
    SDL_ShowCursor();
  }
}

void DebugSavePortContents(const CGrafPort* port, const char* filename) {
  auto* cc_port = CCGrafPort::as_port(port);
  if (port) {
    phosg::save_file(filename, cc_port->data.serialize(phosg::ImageFormat::WINDOWS_BITMAP));
  }
}

void LocalToGlobal(Point* pt) {
  pt->h += qd.thePort->portRect.left;
  pt->v += qd.thePort->portRect.top;
}

void GlobalToLocal(Point* pt) {
  pt->h -= qd.thePort->portRect.left;
  pt->v -= qd.thePort->portRect.top;
}
