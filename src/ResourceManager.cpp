#include <algorithm>
#include <deque>
#include <filesystem>
#include <memory>

#include "ResourceManager.h"
#include <phosg/Filesystem.hh>
#include <phosg/Strings.hh>
#include <resource_file/IndexFormats/Formats.hh>
#include <resource_file/QuickDrawFormats.hh>
#include <resource_file/ResourceFile.hh>
#include <resource_file/ResourceTypes.hh>
#include <resource_file/TextCodecs.hh>

ResourceManager_Rect rect_from_data(StringReader& data) {
  ResourceManager_Rect r;
  r.top = data.get_u16b();
  r.left = data.get_u16b();
  r.bottom = data.get_u16b();
  r.right = data.get_u16b();
  return r;
}

static int16_t resError = noErr;
static PrefixedLogger rm_log("[ResourceManager] ");

class ResourceManager {
public:
  struct File {
    std::string classic_filename; // Filename on classic Mac OS (e.g. ":Data Files:Scenario")
    std::string host_filename; // Filename on host system (e.g. "Data Files/Scenario.rsrc")
    int16_t reference_number;
    std::shared_ptr<const ResourceDASM::ResourceFile> rf;
  };

  ResourceManager() {}

  void print_chain() const {
    rm_log.info("Open files list:");
    for (size_t z = 0; z < this->files.size(); z++) {
      auto f = this->files[z];
      rm_log.info("  %s from %s (ref %hd)%s",
          f->classic_filename.c_str(),
          f->host_filename.c_str(),
          f->reference_number,
          (z == this->search_start_index) ? " (GetResource search starts here)" : "");
    }
  }

  int16_t use(
      const std::string& classic_filename,
      const std::string& host_filename,
      std::shared_ptr<const ResourceDASM::ResourceFile> rf) {
    auto file = std::make_shared<File>();
    file->classic_filename = classic_filename;
    file->host_filename = host_filename;
    file->reference_number = this->next_reference_number++;
    file->rf = rf;
    this->files.insert(this->files.begin(), file);
    return file->reference_number;
  }

  void use(int16_t reference_number) {
    for (size_t z = 0; z < this->files.size(); z++) {
      if (this->files[z]->reference_number == reference_number) {
        this->search_start_index = z;
        break;
      }
    }
  }

  std::shared_ptr<const File> current_file() const {
    if (this->search_start_index >= this->files.size()) {
      throw std::runtime_error("Current resource file is out of bounds");
    }
    return this->files[this->search_start_index];
  }

  std::shared_ptr<const ResourceDASM::ResourceFile::Resource> find_resource(int32_t type, int16_t id) {
    std::string type_str = ResourceDASM::string_for_resource_type(type);
    for (size_t z = this->search_start_index; z < this->files.size(); z++) {
      try {
        return this->files[z]->rf->get_resource(type, id);
      } catch (const std::out_of_range&) {
      }
    }
    rm_log.info("%s:%hd not found in any open resource file", type_str.c_str(), id);
    this->print_chain();
    return nullptr;
  }

private:
  int16_t next_reference_number = 1;
  std::vector<std::shared_ptr<const File>> files;
  size_t search_start_index = 0;
};

static ResourceManager rm;

void* ResourceManager_GetResource(int32_t theType, int16_t theID) {
  auto resource = rm.find_resource(theType, theID);

  resError = resNotFound;
  return NULL;
}

void ResourceManager_get_Str255_from_strN(char* out, int16_t res_id, uint16_t index) {
  try {
    auto res = rm.find_resource(ResourceDASM::RESOURCE_TYPE_STRN, res_id);
    auto decoded = ResourceDASM::ResourceFile::decode_STRN(res);
    const auto& str = decoded.strs.at(index);
    if (str.size() > 0xFF) {
      out[0] = 0; // This should be impossible; the STR# format has a single-byte size field per string
    } else {
      out[0] = str.size();
      memcpy(&out[1], str.data(), str.size());
      memset(&out[str.size() + 1], 0, 0xFF - str.size());
    }
  } catch (const std::out_of_range&) {
    out[0] = 0;
  }
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

ResourceManager_PixPat ResourceManager_get_ppat_resource(int16_t patID) {
  auto resource = rm.find_resource(ResourceDASM::RESOURCE_TYPE_ppat, patID);
  if (!resource) {
    throw std::runtime_error(string_printf("Resource ppat:%hd was not found", patID));
  }
  StringReader r(resource->data.data(), resource->data.size());
  const auto& header = r.get<PixelPatternResourceHeader>();
  const auto& pixmap_header = r.pget<ResourceDASM::PixelMapHeader>(header.pixel_map_offset + 4);
  ResourceManager_PixMap rmPixMap = {
      pixmap_header.pixel_size,
      {pixmap_header.bounds.y1,
          pixmap_header.bounds.x1,
          pixmap_header.bounds.y2,
          pixmap_header.bounds.x2}};

  ResourceDASM::ResourceFile::DecodedPattern pattern = ResourceDASM::ResourceFile::decode_ppat(resource);
  const char* patData = reinterpret_cast<const char*>(pattern.pattern.get_data());

  return {
      header.type,
      rmPixMap,
      &patData};
}

ResourceManager_Picture ResourceManager_get_pict_resource(int16_t picID) {
  auto resource = rm.find_resource(ResourceDASM::RESOURCE_TYPE_PICT, picID);
  ResourceDASM::ResourceFile::DecodedPictResource p = rm.current_file()->rf->decode_PICT(resource);

  // Have to copy the raw data out of the Image object, so that it doesn't get freed
  // out from under us
  void* data = malloc(p.image.get_data_size());
  memcpy(data, p.image.get_data(), p.image.get_data_size());

  return {
      0,
      {0, 0, (int32_t)p.image.get_height(), (int32_t)p.image.get_width()},
      data};
}

ResourceManager_Window ResourceManager_get_wind_resource(int16_t windowID) {
  auto resource = rm.find_resource(ResourceDASM::RESOURCE_TYPE_WIND, windowID);
  StringReader data(resource->data.data(), resource->data.size());

  ResourceManager_Rect r = rect_from_data(data);
  int16_t procID = data.get_s16b();
  bool visible = (bool)data.get_u16b();
  bool dismissable = (bool)data.get_u16b();
  uint32_t refCon = data.get_u32b();
  uint8_t nameSize = data.get_u8();
  std::string n = data.read(nameSize);

  ResourceManager_Window w = {
      r,
      procID,
      visible,
      dismissable,
      refCon};
  w.windowTitle[0] = nameSize;
  strcpy(&w.windowTitle[1], n.c_str());
  return w;
}

ResourceManager_Dialog ResourceManager_get_dlog_resource(int16_t dialogID) {
  auto dlog = rm.find_resource(ResourceDASM::RESOURCE_TYPE_DLOG, dialogID);
  StringReader data(dlog->data.data(), dlog->data.size());

  ResourceManager_Rect r = rect_from_data(data);
  int16_t wDefID = data.get_s16b();
  bool visible = (bool)data.get_u8();
  data.read(1);
  bool dismissable = (bool)data.get_u8();
  data.read(1);
  uint32_t refCon = data.get_u32b();
  int16_t ditlID = data.get_s16b();

  return {
      r,
      wDefID,
      visible,
      dismissable,
      refCon,
      ditlID};
}

uint16_t ResourceManager_get_ditl_resources(int16_t ditlID, ResourceManager_DialogItem** items) {
  auto ditl = rm.find_resource(ResourceDASM::RESOURCE_TYPE_DITL, ditlID);
  StringReader ditlData = StringReader(ditl->data.data(), ditl->data.size());
  uint16_t numItems = ditlData.get_u16b() + 1;
  *items = (ResourceManager_DialogItem*)calloc(numItems, sizeof(ResourceManager_DialogItem));
  for (int i = 0; i < numItems; i++) {
    ditlData.read(4);
    ResourceManager_Rect dispWindow = rect_from_data(ditlData);
    uint8_t type = ditlData.get_u8();
    bool enabled = (bool)(type & 0x80);
    type = (type & 0x7F);
    switch (type) {
      // PICT Resource
      case 64: {
        ditlData.read(1);
        uint16_t pictID = ditlData.get_u16b();
        auto p = ResourceManager_get_pict_resource(pictID);
        (*items)[i].type = (*items)[i].DIALOG_ITEM_TYPE_PICT;
        (*items)[i].dialogItem.pict.dispRect = dispWindow;
        (*items)[i].dialogItem.pict.enabled = enabled;
        (*items)[i].dialogItem.pict.p = p;
        break;
      }
      default:
        break;
    }
  }
  return numItems;
}

ResourceManager_Sound ResourceManager_get_snd_resource(int16_t soundID) {
  auto snd = rm.find_resource(ResourceDASM::RESOURCE_TYPE_snd, soundID);
  ResourceDASM::ResourceFile::DecodedSoundResource soundData = rm.current_file()->rf->decode_snd(snd);
  size_t len = soundData.data.size();
  void* data = malloc(len);
  memcpy(data, soundData.data.data(), len);
  return ResourceManager_Sound{
      .freq = soundData.sample_rate,
      .data = data,
      .len = len};
}

int16_t ResourceManager_OpenResFile(const char classic_filename[256], signed char permission) {
  // TODO(martin): The following behavior is not implemented (but it probably
  // is not necessary for our use case): if OpenResFile() is called again on a
  // file that's already open, it should NOT set the current resource file, and
  // instead just return the reference number of the existing opened file.

  auto base_host_filename = std::string(classic_filename);
  std::replace(base_host_filename.begin(), base_host_filename.end(), ':', '/');

  std::vector<std::string> candidate_host_filenames;
  // Try looking in . first (this is what the compiled game will do)
  candidate_host_filenames.emplace_back(base_host_filename + ".rsrc");
  // Try looking in base/Realmz next (this happens when building in the git repo)
  candidate_host_filenames.emplace_back("base/Realmz/" + base_host_filename + ".rsrc");
  // Finally, try looking in resources/ (this also happens when building in the git repo)
  candidate_host_filenames.emplace_back("resources/" + base_host_filename + ".rsrc");

  for (const auto& candidate_host_filename : candidate_host_filenames) {
    try {
      std::string data = load_file(candidate_host_filename);
      auto rf = std::make_shared<ResourceDASM::ResourceFile>(ResourceDASM::parse_resource_fork(data));
      int16_t ret = rm.use(classic_filename, candidate_host_filename, rf);
      rm_log.info("Loaded %s from %s with reference number %hd", classic_filename, candidate_host_filename.c_str(), ret);
      rm.print_chain();
      return ret;
    } catch (const cannot_open_file&) {
    }
  }

  rm_log.info("Failed to load %s from any known location", classic_filename);
  return -1;
}

void ResourceManager_UseResFile(int16_t reference_number) {
  rm.use(reference_number);
}

int16_t ResourceManager_CurResFile() {
  return rm.current_file()->reference_number;
}
