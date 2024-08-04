#include <algorithm>
#include <deque>
#include <filesystem>
#include <memory>

#include "MemoryManager.hpp"
#include "QuickDraw.h"
#include "ResourceManager.hpp"
#include "Types.hpp"
#include <phosg/Filesystem.hh>
#include <phosg/Strings.hh>
#include <resource_file/IndexFormats/Formats.hh>
#include <resource_file/QuickDrawFormats.hh>
#include <resource_file/ResourceFile.hh>
#include <resource_file/ResourceTypes.hh>
#include <resource_file/TextCodecs.hh>

Rect rect_from_reader(phosg::StringReader& data) {
  Rect r;
  r.top = data.get_u16b();
  r.left = data.get_u16b();
  r.bottom = data.get_u16b();
  r.right = data.get_u16b();
  return r;
}

struct PixelPatternResourceHeader {
  be_uint16_t type;
  be_uint32_t pixel_map_offset;
  be_uint32_t pixel_data_offset;
  be_uint32_t unused1; // TMPL: "Expanded pixel image" (probably ptr to decompressed data when used by QuickDraw)
  be_uint16_t unused2; // TMPL: "Pattern valid flag" (unused in stored resource)
  be_uint32_t reserved; // TMPL: "Expanded pattern"
  uint8_t monochrome_pattern[8];
} __attribute__((packed));

PixPatHandle GetPixPat(uint16_t patID) {
  auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_ppat, patID);
  if (!data_handle) {
    throw std::runtime_error(phosg::string_printf("Resource ppat:%hd was not found", patID));
  }
  auto r = read_from_handle(data_handle);
  const auto& header = r.get<PixelPatternResourceHeader>();

  const auto& pixmap_header = r.pget<ResourceDASM::PixelMapHeader>(header.pixel_map_offset + 4);
  auto patMap = NewHandleTyped<PixMap>();
  (*patMap)->pixelSize = pixmap_header.pixel_size;
  (*patMap)->bounds.top = pixmap_header.bounds.y1;
  (*patMap)->bounds.left = pixmap_header.bounds.x1;
  (*patMap)->bounds.bottom = pixmap_header.bounds.y2;
  (*patMap)->bounds.right = pixmap_header.bounds.x2;

  ResourceDASM::ResourceFile::DecodedPattern pattern = ResourceDASM::ResourceFile::decode_ppat(
      *data_handle, GetHandleSize(data_handle));
  const char* patData = reinterpret_cast<const char*>(pattern.pattern.get_data());

  auto ret_handle = NewHandleTyped<PixPat>();
  auto& ret = **ret_handle;
  ret.patType = header.type;
  ret.patMap = patMap;
  ret.patData = nullptr; // TODO
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

Picture QuickDraw_get_pict_resource(int16_t picID) {
  auto p = ResourceManager_decode_PICT(picID);

  // Have to copy the raw data out of the Image object, so that it doesn't get freed
  // out from under us
  Picture ret;
  ret.picSize = 0; // This is common for Picture objects
  ret.picFrame.top = 0;
  ret.picFrame.left = 0;
  ret.picFrame.bottom = p.image.get_height();
  ret.picFrame.right = p.image.get_width();
  ret.data = NewHandleWithData(p.image.get_data(), p.image.get_data_size());
  return ret;
}
