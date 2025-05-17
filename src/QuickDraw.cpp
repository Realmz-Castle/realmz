#include "QuickDraw.hpp"

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
#include <stdexcept>
#include <unordered_map>

#include "MemoryManager.hpp"
#include "ResourceManager.h"
#include "StringConvert.hpp"
#include "Types.hpp"
#include "WindowManager.hpp"

static phosg::PrefixedLogger qd_log("[QuickDraw] ");
static std::unordered_set<int16_t> already_decoded{};
static std::unordered_map<CGrafPtr, std::shared_ptr<GraphicsCanvas>> canvas_lookup{};

// Originally declared in variables.h. It seems that `qd` was introduced by Myriad during the
// port to PC in place of Classic Mac's global QuickDraw context. We can repurpose it here
// for easier access in our code, while still exposing a C-compatible struct.
QuickDrawGlobals qd{
    .defaultPort = {
        .pnSize = {.v = 1, .h = 1}}};

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
      qd_log.error_f("Unrecognized color constant {}", color_const);
      break;
  }
  return RGBColor{};
}

void register_canvas(std::shared_ptr<GraphicsCanvas> canvas) {
  canvas_lookup.insert({canvas->get_port(), canvas});
}

void deregister_canvas(std::shared_ptr<GraphicsCanvas> canvas) {
  try {
    canvas_lookup.erase(canvas->get_port());
  } catch (std::out_of_range) {
    qd_log.error_f("Tried to delete canvas with id {}, but it wasn't in lookup", reinterpret_cast<const void*>(canvas->get_port()));
  }
}

void deregister_canvas(const CGrafPtr port) {
  try {
    canvas_lookup.erase(port);
  } catch (std::out_of_range) {
    qd_log.error_f("Tried to delete canvas with id {}, but it wasn't in lookup", reinterpret_cast<const void*>(port));
  }
}

std::shared_ptr<GraphicsCanvas> lookup_canvas(CGrafPtr port) {
  try {
    return canvas_lookup.at(port);
  } catch (std::out_of_range) {
    throw std::runtime_error("Could not find canvas for given id");
  }
}

std::shared_ptr<GraphicsCanvas> current_canvas() {
  return lookup_canvas(qd.thePort);
}

void draw_rect(const Rect& dispRect) {
  current_canvas()->draw_rect(dispRect);
}

void draw_rgba_picture(void* pixels, int w, int h, const Rect& rect) {
  current_canvas()->draw_rgba_picture(pixels, w, h, rect);
}

void set_draw_color(const RGBColor& color) {
  current_canvas()->set_draw_color(color);
}

void draw_line(const Point& start, const Point& end) {
  current_canvas()->draw_line(start, end);
}

void draw_text(const std::string& text) {
  auto canvas = current_canvas();

  canvas->draw_text(text);
}

int measure_text(const std::string& text) {
  return current_canvas()->measure_text(text);
}

void render_current_canvas(const SDL_FRect* rect) {
  current_canvas()->render(rect);
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
  // satisfy the above, we replace that with the fully decoded Picture resource.
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_PICT, id);
  if (!data_handle) {
    return nullptr;
  }

  if (already_decoded.contains(id)) {
    return reinterpret_cast<PicHandle>(data_handle);
  }

  auto p = ResourceDASM::ResourceFile::decode_PICT_only(*data_handle, GetHandleSize(data_handle));

  if (p.image.get_height() == 0 || p.image.get_width() == 0) {
    throw std::runtime_error(std::format("Failed to decode PICT {}", id));
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
  add_destroy_callback(data_handle, [id]() -> void {
    already_decoded.erase(id);
  });

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
void InitGraf(QuickDrawGlobals* new_globals) {
  qd.thePort = &qd.defaultPort;
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
  (*h)->iconBMap.bounds = Rect{
      0,
      0,
      static_cast<int16_t>(decoded_cicn.image.get_height()),
      static_cast<int16_t>(decoded_cicn.image.get_width())};
  (*h)->iconData = NewHandleWithData(decoded_cicn.image.get_data(), decoded_cicn.image.get_data_size());
  return h;
}

OSErr DisposeCIcon(CIconHandle icon) {
  if ((*icon)->iconData) {
    DisposeHandle((*icon)->iconData);
  }
  DisposeHandleTyped(icon);
  return noErr;
}

OSErr PlotCIcon(const Rect* theRect, CIconHandle theIcon) {
  auto bounds = (*theIcon)->iconBMap.bounds;
  int w = bounds.right - bounds.left;
  int h = bounds.bottom - bounds.top;
  draw_rgba_picture(
      *((*theIcon)->iconData),
      w, h, *theRect);
  render_current_canvas(NULL);

  render_window(qd.thePort);

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
}

void PenSize(int16_t width, int16_t height) {
  qd.thePort->pnSize = {height, width};
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

// Internally, we represent an offscreen GWorld as a windowless GraphicsCanvas, which by default
// will use a software renderer and draw to an in-memory buffer rather than the GPU.
QDErr NewGWorld(GWorldPtr* offscreenGWorld, int16_t pixelDepth, const Rect* boundsRect, CTabHandle cTable,
    GDHandle aGDevice, GWorldFlags flags) {
  auto portRecord = *qd.thePort;
  portRecord.portRect = *boundsRect;
  auto canvas = std::make_shared<GraphicsCanvas>(portRecord);
  canvas->init();
  register_canvas(canvas);

  *offscreenGWorld = canvas->get_port();

  return 0;
}

void DisposeGWorld(GWorldPtr offscreenWorld) {
  deregister_canvas(offscreenWorld);
}

void DrawString(ConstStr255Param s) {
  auto str = string_for_pstr<255>(s);

  draw_text(str);
  render_current_canvas(NULL);

  render_window(qd.thePort);
}

int16_t TextWidth(const void* textBuf, int16_t firstByte, int16_t byteCount) {
  // Realmz always calls this procedure with 0 as the first byte, and the full
  // strlen as the byteCount, so we can ignore those parameters and just measure
  // the full string.
  // Realmz also seems to only call this with cstrings, so we're good there as well.
  return current_canvas()->measure_text(static_cast<const char*>(textBuf));
}

void DrawPicture(PicHandle myPicture, const Rect* dstRect) {
  auto picFrame = (*myPicture)->picFrame;
  int w = picFrame.right - picFrame.left;
  int h = picFrame.bottom - picFrame.top;

  draw_rgba_picture(*((*myPicture)->data), w, h, *dstRect);
  render_current_canvas(NULL);
  render_window(qd.thePort);
}

void LineTo(int16_t h, int16_t v) {
  CGrafPtr port = qd.thePort;

  set_draw_color(port->rgbBgColor);
  draw_line(port->pnLoc, {v, h});
  port->pnLoc = {v, h};
  render_current_canvas(NULL);
  render_window(qd.thePort);
}

void FrameOval(const Rect* r) {
  CGrafPtr port = qd.thePort;

  set_draw_color(port->rgbBgColor);
  current_canvas()->draw_oval(*r);
  render_current_canvas(NULL);
  render_window(qd.thePort);
}

void CopyBits(const BitMap* srcBits, const BitMap* dstBits, const Rect* srcRect, const Rect* dstRect, int16_t mode,
    RgnHandle maskRgn) {
  auto srcCanvas = lookup_canvas(const_cast<CGrafPtr>(reinterpret_cast<const CGrafPort*>(srcBits)));
  auto dstCanvas = lookup_canvas(const_cast<CGrafPtr>(reinterpret_cast<const CGrafPort*>(dstBits)));

  dstCanvas->copy_from(*srcCanvas, *srcRect, *dstRect);
  dstCanvas->render(NULL);
  render_window(dstCanvas->get_port());
}

void CopyMask(const BitMap* srcBits, const BitMap* maskBits, const BitMap* dstBits, const Rect* srcRect, const Rect* maskRect,
    const Rect* dstRect) {
}

void EraseRect(const Rect* r) {
  current_canvas()->clear_rect(*r);
  render_window(qd.thePort);
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
      SDL_PIXELFORMAT_RGBA32,
      const_cast<void*>(cursor.decoded->image.get_data()),
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
