#include <algorithm>
#include <deque>
#include <filesystem>
#include <memory>
#include <stddef.h>

#include "FileManager.hpp"
#include "MemoryManager.hpp"
#include "ResourceManager.h"
#include "StringConvert.hpp"
#include "structs.h"
#include <phosg/Filesystem.hh>
#include <phosg/Strings.hh>
#include <resource_file/IndexFormats/Formats.hh>
#include <resource_file/QuickDrawFormats.hh>
#include <resource_file/ResourceFile.hh>
#include <resource_file/ResourceTypes.hh>
#include <resource_file/TextCodecs.hh>

////////////////////////////////////////////////////////////////////////////////
// Internal Resource Manager implementation

static int16_t resError = noErr;
static phosg::PrefixedLogger rm_log("[ResourceManager] ");

class ResourceManager {
public:
  struct Resource {
    int16_t file_refnum = 0;
    std::shared_ptr<const ResourceDASM::ResourceFile::Resource> source_res;
    Handle data_handle = nullptr;
    // This field specifies if the data pointed to by data_handle doesn't match
    // the data contained in source_res. If this is true, the data is copied
    // into source_res when the file is saved.
    bool data_modified;

    ~Resource() {
      if (this->data_handle) {
        DisposeHandle(this->data_handle);
      }
    }
  };

  struct File {
    enum class State {
      READ_ONLY = 0,
      NOT_MODIFIED,
      MODIFIED,
    };
    std::string host_filename; // Filename on host system (e.g. "Data Files/Scenario.rsrc")
    State state;
    int16_t refnum = 0;
    std::shared_ptr<ResourceDASM::ResourceFile> rf;

    std::unordered_map<uint64_t, std::shared_ptr<Resource>> resource_for_type_id;
    std::unordered_map<Handle, std::shared_ptr<Resource>> resource_for_handle;

    static uint64_t key_for_type_id(uint32_t type, int16_t id) {
      return static_cast<uint64_t>(type) | (static_cast<uint64_t>(id) << 32);
    }

    std::shared_ptr<Resource> get_resource(uint32_t type, int16_t id) {
      uint64_t key = this->key_for_type_id(type, id);
      auto loaded_res = this->resource_for_type_id.find(key);
      if (loaded_res != this->resource_for_type_id.end()) {
        return loaded_res->second;
      }
      std::shared_ptr<const ResourceDASM::ResourceFile::Resource> source_res;
      try {
        source_res = this->rf->get_resource(type, id);
      } catch (const std::out_of_range&) {
        return nullptr;
      }
      auto res = std::make_shared<Resource>();
      res->file_refnum = this->refnum;
      res->source_res = source_res;
      res->data_handle = NewHandleWithData(res->source_res->data);
      res->data_modified = false;
      this->resource_for_type_id.emplace(key, res);
      this->resource_for_handle.emplace(res->data_handle, res);
      return res;
    }

    std::shared_ptr<Resource> get_resource(Handle data_handle) {
      auto res = this->resource_for_handle.find(data_handle);
      if (res != this->resource_for_handle.end()) {
        return res->second;
      } else {
        return nullptr;
      }
    }

    void mark_modified(Handle data_handle) {
      // Marks the file modified, and (optionally) the given resource as well
      if (this->state == File::State::READ_ONLY) {
        throw std::logic_error("Cannot mark read-only resource file as modified");
      }
      if (data_handle) {
        auto res = this->get_resource(data_handle);
        if (res == nullptr) {
          throw std::logic_error("Tried to mark unknown data handle as modified");
        }
        res->data_modified = true;
      }
      this->state = ResourceManager::File::State::MODIFIED;
    }

    std::shared_ptr<Resource> add_resource(uint32_t type, int16_t id, std::string&& name, Handle data_handle) {
      if (this->rf->resource_exists(type, id)) {
        throw std::invalid_argument(phosg::string_printf("Resource %08" PRIX32 ":%hd already exists", type, id));
      }

      this->mark_modified(nullptr); // Throws if the file is read-only

      // Create the backing objects in ResourceFile, so we can save the changes
      // later
      auto data_str = string_for_handle(data_handle);
      const void* data = *data_handle;
      auto rf_res = make_shared<ResourceDASM::ResourceFile::Resource>(type, id, 0, std::move(name), std::move(data_str));
      this->rf->add(rf_res);

      // data_handle is now owned by the Resource Manager, so track it as a
      // loaded resource. We can't just free it since the application can still
      // use it even though we're responsible for freeing it now
      auto res = std::make_shared<Resource>();
      res->file_refnum = this->refnum;
      res->source_res = rf_res;
      res->data_handle = data_handle;
      res->data_modified = false;
      if (!this->resource_for_type_id.emplace(this->key_for_type_id(type, id), res).second) {
        throw std::logic_error(phosg::string_printf(
            "Added resource %08" PRIX32 ":%hd but it already exists in the ResourceFile",
            type, id));
      }
      if (!this->resource_for_handle.emplace(res->data_handle, res).second) {
        throw std::logic_error(phosg::string_printf(
            "Added resource %08" PRIX32 ":%hd but its handle is already owned by the Resource Manager",
            type, id));
      }

      return res;
    }

    void remove_resource(std::shared_ptr<Resource> res) {
      this->mark_modified(nullptr); // Throws if the file is read-only

      // data_handle is not freed; the application must do this manually if it
      // doesn't want to keep the data in memory.
      this->resource_for_type_id.erase(this->key_for_type_id(res->source_res->type, res->source_res->id));
      this->resource_for_handle.erase(res->data_handle);
      res->data_handle = nullptr; // Prevent destructor from freeing the handle

      // Delete the backing object in ResourceFile
      this->rf->remove(res->source_res->type, res->source_res->id);
    }

    void release_resource(std::shared_ptr<Resource> res, bool free_data) {
      // According to Inside Macintosh, both ReleaseResource and DetachResource
      // return noErr but do nothing if called on a resource that has been
      // modified and not yet written to disk.
      if (res->data_modified) {
        return;
      }

      this->resource_for_type_id.erase(this->key_for_type_id(res->source_res->type, res->source_res->id));
      this->resource_for_handle.erase(res->data_handle);
      if (!free_data) {
        res->data_handle = nullptr; // Prevent destructor from freeing the handle
      }
    }

    void write() {
      if (this->state != State::MODIFIED) {
        return;
      }
      // For all resources that were marked modified, copy their data into the
      // backing ResourceFile::Resource objects before serializing the resource
      // index and writing to disk
      for (auto& it : this->resource_for_type_id) {
        if (it.second->data_modified) {
          auto src_res = it.second->source_res;
          auto new_data = string_for_handle(it.second->data_handle);
          auto new_res = make_shared<ResourceDASM::ResourceFile::Resource>(
              src_res->type, src_res->id, src_res->flags, src_res->name, new_data);
          this->rf->remove(src_res->type, src_res->id);
          this->rf->add(new_res);
          it.second->source_res = new_res;
          it.second->data_modified = false;
        }
      }
      phosg::save_file(this->host_filename, ResourceDASM::serialize_resource_fork(*this->rf));
      this->state = State::NOT_MODIFIED;
    }
  };

  ResourceManager() = default;
  ~ResourceManager() = default;

  void print_chain() const {
    rm_log.info("Open files list:");
    for (size_t z = 0; z < this->files.size(); z++) {
      auto f = this->files[z];
      rm_log.info("  %s (ref %hd)%s",
          f->host_filename.c_str(),
          f->refnum,
          (z == this->search_start_index) ? " (GetResource search starts here)" : "");
    }
  }

  int16_t use(
      const std::string& host_filename,
      std::shared_ptr<ResourceDASM::ResourceFile> rf,
      bool writable) {
    auto file = std::make_shared<File>();
    file->host_filename = host_filename;
    file->refnum = this->next_refnum++;
    file->state = writable ? File::State::NOT_MODIFIED : File::State::READ_ONLY;
    file->rf = rf;
    this->files.insert(this->files.begin(), file);
    return file->refnum;
  }

  void use(int16_t refnum) {
    for (size_t z = 0; z < this->files.size(); z++) {
      if (this->files[z]->refnum == refnum) {
        this->search_start_index = z;
        break;
      }
    }
  }

  void close(int16_t refnum) {
    auto file_it = this->files.end();
    for (auto it = this->files.begin(); it != this->files.end(); it++) {
      if ((*it)->refnum == refnum) {
        file_it = it;
        break;
      }
    }
    if (file_it != this->files.end()) {
      (*file_it)->write();
      this->files.erase(file_it);
    }
  }

  const std::vector<std::shared_ptr<File>>& chain() {
    return this->files;
  }

  std::shared_ptr<File> current_file() const {
    if (this->search_start_index >= this->files.size()) {
      throw std::runtime_error("Current resource file is out of bounds");
    }
    return this->files[this->search_start_index];
  }

  std::shared_ptr<File> get_file(int16_t refnum) const {
    for (auto f : this->files) {
      if (f->refnum == refnum) {
        return f;
      }
    }
    throw std::out_of_range(phosg::string_printf("Resource file with refnum %hd is not open", refnum));
  }

  std::shared_ptr<Resource> get_resource(int32_t type, int16_t id) {
    if (type == ResourceDASM::RESOURCE_TYPE_snd && id == 0) {
      // TODO: Apparently, snd 0 should be provided by the system resource fork. In fact,
      // snd resources 0-8191 are reserved (Sound 2-154), but Realmz seems to use many of
      // these.
      return nullptr;
    }
    for (size_t z = this->search_start_index; z < this->files.size(); z++) {
      auto res = this->files[z]->get_resource(type, id);
      if (res != nullptr) {
        return res;
      }
    }
    std::string type_str = ResourceDASM::string_for_resource_type(type);
    rm_log.info("%s:%hd not found in any open resource file", type_str.c_str(), id);
    this->print_chain();
    throw std::out_of_range(phosg::string_printf("resource %s:%hd not found", type_str.c_str(), id));
  }

  std::shared_ptr<Resource> get_resource(Handle data_handle) {
    for (size_t z = this->search_start_index; z < this->files.size(); z++) {
      auto res = this->files[z]->get_resource(data_handle);
      if (res != nullptr) {
        return res;
      }
    }
    rm_log.info("Handle not found in any open resource file");
    this->print_chain();
    throw std::out_of_range("resource not found by handle");
  }

  void mark_modified(int16_t refnum) {
    this->get_file(refnum)->mark_modified(nullptr);
  }

  void mark_modified(Handle data_handle) {
    auto res = this->get_resource(data_handle);
    this->get_file(res->file_refnum)->mark_modified(data_handle);
  }

  std::shared_ptr<Resource> add_resource(uint32_t type, int16_t id, std::string&& name, Handle data_handle) {
    // data_handle must not be null and must not be owned by another resource
    if (!data_handle) {
      throw std::invalid_argument("data_handle is null");
    }
    try {
      this->get_resource(data_handle);
      throw std::invalid_argument("data_handle already belongs to another resource");
    } catch (const std::out_of_range&) {
    }

    return this->current_file()->add_resource(type, id, std::move(name), data_handle);
  }

  std::shared_ptr<Resource> remove_resource(Handle data_handle) {
    // data_handle must not be null and must be owned by a resource
    if (!data_handle) {
      throw std::invalid_argument("data_handle is null");
    }
    auto res = this->get_resource(data_handle);
    this->get_file(res->file_refnum)->remove_resource(res);
    return res;
  }

  void release_resource(Handle data_handle, bool free_data) {
    auto res = this->get_resource(data_handle);
    this->get_file(res->file_refnum)->release_resource(res, free_data);
  }

  void write_file(int16_t refnum) {
    this->get_file(refnum)->write();
  }

private:
  int16_t next_refnum = 1;
  std::vector<std::shared_ptr<File>> files;
  size_t search_start_index = 0;
};

static ResourceManager rm;

////////////////////////////////////////////////////////////////////////////////
// Classic Mac OS API implementation

std::string host_resource_filename_for_host_filename(const std::string& host_path, bool allow_missing = false) {
  std::string full_path = host_path + ".rsrc";
  if (allow_missing || phosg::isfile(full_path)) {
    return full_path;
  }

  return "";
}

std::string host_resource_filename_for_mac_filename(const std::string& host_path, bool allow_missing = false) {
  return host_resource_filename_for_host_filename(host_filename_for_mac_filename(host_path, false), allow_missing);
}

std::string host_resource_filename_for_FSSpec(const FSSpec* fsp, bool allow_missing = false) {
  return host_resource_filename_for_host_filename(host_filename_for_FSSpec(fsp), allow_missing);
}

void FSpCreateResFile(const FSSpec* spec, OSType creator, OSType fileType, ScriptCode scriptTag) {
  // It seems that this function is the equivalent of the touch command on
  // Unix-like systems: it just creates the file if it doesn't exist, and
  // doesn't open it. (The caller must call OpenResFile if they actually want
  // to open it.)

  // We ignore Finder info and just create the file itself.
  (void)creator;
  (void)fileType;
  (void)scriptTag;

  auto filename = host_resource_filename_for_FSSpec(spec, true);
  if (filename.empty()) {
    throw std::runtime_error("Cannot create resource file in any known location");
  }
  int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666);
  if (fd >= 0) {
    close(fd);
  }

  resError = noErr;
}

int16_t FSpOpenResFile(const FSSpec* spec, SInt8 permission) {
  auto host_filename = host_resource_filename_for_FSSpec(spec);
  if (host_filename.empty()) {
    auto filename = string_for_pstr<64>(spec->name);
    rm_log.info("Failed to load resource file %s", filename.c_str());
    resError = fnfErr;
    return -1;
  }

  try {
    std::string data = phosg::load_file(host_filename);
    auto rf = std::make_shared<ResourceDASM::ResourceFile>(ResourceDASM::parse_resource_fork(data));
    bool writable = (permission == fsCurPerm) || (permission > fsRdPerm);
    int16_t ret = rm.use(host_filename, rf, writable);
    rm_log.info("Loaded %s with reference number %hd (%s with permission %hhd)",
        host_filename.c_str(), ret, writable ? "writable" : "read-only", permission);
    rm.print_chain();
    resError = noErr;
    return ret;

  } catch (const phosg::cannot_open_file&) {
    rm_log.info("Failed to load resource file %s", host_filename.c_str());
    resError = fnfErr;
    return -1;
  }
}

void CloseResFile(int16_t refnum) {
  UpdateResFile(refnum);
  rm.close(refnum);
  resError = noErr;
  rm_log.info("Closed file reference number %hd", refnum);
  rm.print_chain();
}

int16_t ResError(void) {
  return resError;
}

UInt16 CurResFile(void) {
  return rm.current_file()->refnum;
}

void UseResFile(int16_t refnum) {
  rm.use(refnum);
  resError = noErr;
}

int16_t CountResources(ResType theType) {
  int16_t ret = 0;
  for (const auto& f : rm.chain()) {
    ret += f->rf->count_resources_of_type(theType);
  }
  return ret;
}

void GetResInfo(Handle data_handle, int16_t* res_id, ResType* res_type, Str255 res_name) {
  try {
    auto res = rm.get_resource(data_handle);
    *res_id = res->source_res->id;
    *res_type = res->source_res->type;
    pstr_for_string<256>(res_name, res->source_res->name);
    resError = noErr;
  } catch (const std::out_of_range&) {
    resError = resNotFound;
  }
}

int16_t GetResAttrs(Handle data_handle) {
  try {
    auto res = rm.get_resource(data_handle);
    resError = noErr;
    // Only the low 8 bits are actually part of the Resource Manager; the
    // others are used internally by resource_dasm so we mask them out here
    return res->source_res->flags & 0xFF;
  } catch (const std::out_of_range&) {
    resError = resNotFound;
    return resNotFound;
  }
}

void SetResAttrs(Handle data_handle, int16_t attrs) {
  try {
    auto res = rm.get_resource(data_handle);
    resError = noErr;
    rm_log.info("Skipping SetResAttrs on resource %08" PRIX32 ":%hd (value=%04X)",
        res->source_res->type, res->source_res->id, attrs);
  } catch (const std::out_of_range&) {
    resError = resNotFound;
    rm_log.info("Skipping SetResAttrs on missing resource");
  }
}

void AddResource(Handle data_handle, ResType type, int16_t id, ConstStr255Param name) {
  auto name_str = string_for_pstr<256>(name);
  try {
    rm.add_resource(type, id, std::move(name_str), data_handle);
    rm_log.info("Resource %08" PRIX32 ":%hd added", type, id);
  } catch (const std::exception& e) {
    resError = addResFailed;
    rm_log.info("Failed to add resource %08" PRIX32 ":%hd: %s", type, id, e.what());
  }
}

void RemoveResource(Handle data_handle) {
  try {
    auto res = rm.remove_resource(data_handle);
    resError = noErr;
    rm_log.info("Resource %08" PRIX32 ":%hd deleted", res->source_res->type, res->source_res->id);

  } catch (const std::exception& e) {
    resError = addResFailed;
    rm_log.info("Failed to delete resource: %s", e.what());
  }
}

void ChangedResource(Handle data_handle) {
  try {
    auto res = rm.get_resource(data_handle);
    rm.mark_modified(data_handle);
    resError = noErr;
    rm_log.info("Resource %08" PRIX32 ":%hd marked modified", res->source_res->type, res->source_res->id);

  } catch (const std::out_of_range&) {
    resError = resNotFound;
    rm_log.info("Resource cannot be marked modified because the handle is not owned by the Resource Manager");
  }
}

void WriteResource(Handle data_handle) {
  // We don't have a good way to just write a single resource out, so we
  // instead write everything. This is probably fine because Realmz doesn't
  // seem to call this function in a loop anywhere, so hopefully not much work
  // is wasted by being lazy in this implementation.
  try {
    auto res = rm.get_resource(data_handle);
    UpdateResFile(res->file_refnum); // Sets resError
  } catch (const std::out_of_range& e) {
    resError = resNotFound;
  }
}

void UpdateResFile(int16_t refnum) {
  try {
    rm.write_file(refnum);
    rm_log.info("Wrote resource file %hd", refnum);
    resError = noErr;
  } catch (const std::exception& e) {
    rm_log.info("Failed to write resource file %hd: %s", refnum, e.what());
    resError = resFNotFound;
  }
}

Handle GetResource(ResType type, int16_t id) {
  auto res = rm.get_resource(type, id);
  if (res != nullptr) {
    resError = noErr;
    return res->data_handle;
  }

  resError = resNotFound;
  return nullptr;
}

Handle Get1Resource(ResType type, int16_t id) {
  auto res = rm.current_file()->get_resource(type, id);
  if (res != nullptr) {
    resError = noErr;

    // The default PRFN resource included with Realmz is 64 bytes long. Its data handle is cast
    // to PrefHandle in pref.c, so that it can be accessed as a PrefRecord struct. However,
    // the name_str member of that struct is a Pascal-style string. This means that, in the
    // resource file, it is serialized as a variable length data type. Specifically, it
    // appears as just two bytes in the resource file, a 1 and the ASCII character '0'.
    // In the definition of the PrefRecord struct, though, we use a fixed-length char[255] type
    // to hold the name_str field. This means that, when Realmz casts the 64 byte long data handle
    // to a PrefHandle and tries to access any of its members beyond the first byte of name_str, a
    // heap overflow access error occurs. To avoid this, we resize the data handle.
    if (type == 'PRFN' && id == 128 /* PREF_RES_ID */) {
      SetHandleSize(res->data_handle, sizeof(PrefRecord));
      auto pref_handle = (PrefHandle)res->data_handle;
      int name_str_size = (int)(*pref_handle)->name_str[0];
      auto end_of_name_str = (char*)*pref_handle + offsetof(PrefRecord, name_str) + 1 + name_str_size;
      memmove((char*)*pref_handle + offsetof(PrefRecord, autonote), end_of_name_str, 18);
    }

    return res->data_handle;
  }

  resError = resNotFound;
  return nullptr;
}

int32_t GetResourceSizeOnDisk(Handle data_handle) {
  auto res = rm.get_resource(data_handle);
  if (res != nullptr) {
    resError = noErr;
    return GetHandleSize(data_handle);
  }

  resError = resNotFound;
  return -1;
}

void ReleaseResource(Handle data_handle) {
  rm.release_resource(data_handle, true);
}

void DetachResource(Handle data_handle) {
  rm.release_resource(data_handle, false);
}

void GetIndString(Str255 out, int16_t res_id, uint16_t index) {
  try {
    auto res = rm.get_resource(ResourceDASM::RESOURCE_TYPE_STRN, res_id);
    if (res == nullptr) {
      out[0] = 0;
      resError = resNotFound;
      return;
    }
    auto decoded = ResourceDASM::ResourceFile::decode_STRN(res->source_res);
    const auto& str = decoded.strs.at(index - 1);
    if (str.size() > 0xFF) {
      resError = resNotFound;
      out[0] = 0; // This should be impossible; the STR# format has a single-byte size field per string
    } else {
      out[0] = str.size();
      memcpy(&out[1], str.data(), str.size());
      memset(&out[str.size() + 1], 0, 0xFF - str.size());
      resError = noErr;
    }
  } catch (const std::out_of_range&) {
    resError = resNotFound;
    out[0] = 0;
  }
}
