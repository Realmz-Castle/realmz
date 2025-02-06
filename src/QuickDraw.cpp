#include "QuickDraw.h"

#include <SDL3/SDL_pixels.h>

#include <algorithm>
#include <deque>
#include <filesystem>
#include <memory>
#include <phosg/Filesystem.hh>
#include <phosg/Strings.hh>
#include <resource_file/IndexFormats/Formats.hh>
#include <resource_file/QuickDrawFormats.hh>
#include <resource_file/ResourceFile.hh>
#include <resource_file/ResourceFormats.hh>
#include <resource_file/ResourceTypes.hh>
#include <resource_file/TextCodecs.hh>

#include "MemoryManager.hpp"
#include "ResourceManager.h"
#include "Types.hpp"

static phosg::PrefixedLogger qd_log("[QuickDraw] ");
static std::unordered_set<int16_t> already_decoded;
static QuickDrawGlobals* globals = nullptr;

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
      qd_log.error("Unrecognized color constant %d", color_const);
      break;
  }
  return RGBColor{};
}

PixPatHandle GetPixPat(uint16_t patID) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_ppat, patID);
  if (!data_handle) {
    throw std::runtime_error(phosg::string_printf("Resource ppat:%hd was not found", patID));
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

  // Our pattern drawing code expects ppat image data to be RGB24. We want to know if
  // this doesn't turn out to be the case, perhaps in a scenario's resource fork data
  if (pattern.pattern.get_has_alpha()) {
    throw std::logic_error("Decoded ppat image has alpha channel");
  }

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

PicHandle GetPicture(int16_t id) {
  // The GetPicture Mac Classic syscall must return a Handle to a decoded Picture resource,
  // but it must also be the same Handle we use to index loaded Resources in the ResourceManager.
  // Otherwise, subsequent calls to DetachResource or ReleaseResource would fail to find it.
  //
  // By default, the GetResource call leaves the raw bytes of the resource in data_handle. To
  // satisfy the above, we replace that with the fully decoded Picture resource.
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_PICT, id);

  if (already_decoded.contains(id)) {
    return reinterpret_cast<PicHandle>(data_handle);
  }

  auto p = ResourceDASM::ResourceFile::decode_PICT_only(*data_handle, GetHandleSize(data_handle));

  if (p.image.get_height() == 0 || p.image.get_width() == 0) {
    throw std::runtime_error(phosg::string_printf("Failed to decode PICT %hd", id));
  }

  // Normalize all image data to have an alpha channel, for convenience
  p.image.set_has_alpha(true);

  // Have to copy the raw data out of the Image object, so that it doesn't get
  // freed out from under us
  auto ret = NewHandleTyped<Picture>();
  (*ret)->picSize = 0; // This is common for Picture objects
  (*ret)->picFrame.top = 0;
  (*ret)->picFrame.left = 0;
  (*ret)->picFrame.bottom = p.image.get_height();
  (*ret)->picFrame.right = p.image.get_width();
  (*ret)->data = NewHandleWithData(p.image.get_data(), p.image.get_data_size());

  // Now, free the original data handle buffer with the raw bytes, and change the data_handle
  // to contain the new pointer to the decoded image.
  ReplaceHandle(data_handle, reinterpret_cast<Handle>(ret));

  already_decoded.emplace(id);

  return reinterpret_cast<PicHandle>(data_handle);
}

void ForeColor(int32_t color) {
  globals->thePort->fgColor = color;
  globals->thePort->rgbFgColor = color_const_to_rgb(color);
}

void BackColor(int32_t color) {
  globals->thePort->bgColor = color;
  globals->thePort->rgbBgColor = color_const_to_rgb(color);
}

uint32_t rgba8888_for_rgb_color(const RGBColor& color) {
  return (
      (((color.red / 0x0101) & 0xFF) << 24) |
      (((color.green / 0x0101) & 0xFF) << 16) |
      (((color.blue / 0x0101) & 0xFF) << 8) |
      0xFF);
}

SDL_Color sdl_color_for_rgb_color(const RGBColor& color) {
  return SDL_Color{
      static_cast<uint8_t>(color.red / 0x0101),
      static_cast<uint8_t>(color.green / 0x0101),
      static_cast<uint8_t>(color.blue / 0x0101),
      0xFF};
}

void GetBackColor(RGBColor* color) {
  *color = globals->thePort->rgbBgColor;
}
uint32_t GetBackColorRGBA8888() {
  return rgba8888_for_rgb_color(globals->thePort->rgbBgColor);
}
SDL_Color GetBackColorSDL() {
  return sdl_color_for_rgb_color(globals->thePort->rgbBgColor);
}

void GetForeColor(RGBColor* color) {
  *color = globals->thePort->rgbFgColor;
}
uint32_t GetForeColorRGBA8888() {
  return rgba8888_for_rgb_color(globals->thePort->rgbFgColor);
}
SDL_Color GetForeColorSDL() {
  return sdl_color_for_rgb_color(globals->thePort->rgbFgColor);
}

void SetPort(CGrafPtr port) {
  globals->thePort = port;
}

void InitGraf(QuickDrawGlobals* new_globals) {
  globals = new_globals;

  globals->default_graf_handle = NewHandleTyped<CGrafPort>();
  globals->thePort = *globals->default_graf_handle;
  memset(globals->thePort, 0, sizeof(*globals->thePort));
}

CGrafPtr get_default_quickdraw_port() {
  return *globals->default_graf_handle;
}

void TextFont(uint16_t font) {
  globals->thePort->txFont = font;
}

void TextMode(int16_t mode) {
  globals->thePort->txMode = mode;
}

void TextSize(uint16_t size) {
  globals->thePort->txSize = size;
}

void TextFace(int16_t face) {
  globals->thePort->txFace = face;
}

void GetPort(GrafPtr* port) {
  *port = reinterpret_cast<GrafPtr>(globals->thePort);
}

void RGBBackColor(const RGBColor* color) {
  globals->thePort->rgbBgColor = *color;
}

void RGBForeColor(const RGBColor* color) {
  globals->thePort->rgbFgColor = *color;
}

CIconHandle GetCIcon(uint16_t iconID) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_cicn, iconID);
  auto decoded_cicn = ResourceDASM::ResourceFile::decode_cicn(*data_handle, GetHandleSize(data_handle));

  CIconHandle h = NewHandleTyped<CIcon>();
  (*h)->iconBMap.bounds = Rect{
      0,
      0,
      static_cast<int16_t>(decoded_cicn.image.get_height()),
      static_cast<int16_t>(decoded_cicn.image.get_width())};
  (*h)->iconData = NewHandleWithData(decoded_cicn.image.get_data(), decoded_cicn.image.get_data_size());
  return h;
}

void BackPixPat(PixPatHandle ppat) {
  globals->thePort->bkPixPat = ppat;
}

void MoveTo(int16_t h, int16_t v) {
  globals->thePort->pnLoc = Point{v, h};
}

void InsetRect(Rect* r, int16_t dh, int16_t dv) {
  r->left += dh;
  r->right -= dh;
  r->top += dv;
  r->bottom -= dv;
}

void PenPixPat(PixPatHandle ppat) {
  globals->thePort->pnPixPat = ppat;
}

void GetGWorld(CGrafPtr* port, GDHandle* gdh) {
}

PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld) {
  return NULL;
}

void SetGWorld(CGrafPtr port, GDHandle gdh) {
}

QDErr NewGWorld(GWorldPtr* offscreenGWorld, int16_t pixelDepth, const Rect* boundsRect, CTabHandle cTable,
    GDHandle aGDevice, GWorldFlags flags) {
  *offscreenGWorld = (GWorldPtr)malloc(sizeof(CGrafPort));
  (*offscreenGWorld)->portRect.top = boundsRect->top;
  (*offscreenGWorld)->portRect.left = boundsRect->left;
  (*offscreenGWorld)->portRect.bottom = boundsRect->bottom;
  (*offscreenGWorld)->portRect.right = boundsRect->right;

  return 0;
}
void DisposeGWorld(GWorldPtr offscreenWorld) {
}