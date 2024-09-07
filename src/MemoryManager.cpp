#include "MemoryManager.h"

#include <phosg/Strings.hh>

using namespace std;

constexpr int16_t memFullErr = -108;
constexpr int16_t memWZErr = -111;

class MemoryManager : public std::enable_shared_from_this<MemoryManager> {
public:
  struct BlockMeta {
    void* data = nullptr;
    size_t size = 0;
    std::weak_ptr<MemoryManager> memory_manager;

    ~BlockMeta() {
      if (this->data) {
        free(this->data);
      }
    }
  };

  struct HandleMeta {
    enum Flag {
      // The low 8 bits are defined in Mac OS docs. The higher flags are used by
      // this MemoryManager implementation and are not exposed to the calling
      // application.
      LOCKED = 0x80,
      PURGEABLE = 0x40,
      IS_RESOURCE = 0x20,
      // The VALID flag means that the handle is exposed to the client
      // application. Handles can be valid but have a zero size or null data
      // pointer (e.g. after an EmptyHandle call), so it doesn't suffice to
      // just check the data field for this.
      VALID = 0x8000,
    };
    void* data = nullptr;
    size_t size = 0;
    uint16_t flags = 0;
    std::weak_ptr<MemoryManager> memory_manager;

    ~HandleMeta() {
      if (this->data) {
        free(this->data);
      }
    }
  };

  MemoryManager() = default;
  ~MemoryManager() = default;

  void* alloc_block(size_t size) {
    auto meta = make_shared<BlockMeta>();
    meta->data = malloc(size);
    if (!meta->data) {
      this->last_error = memFullErr;
      return nullptr;
    }
    meta->size = size;
    meta->memory_manager = this->weak_from_this();
    this->meta_for_block.emplace(meta->data, meta);
    this->last_error = noErr;
    return meta->data;
  }

  void free_block(void* data) {
    this->last_error = this->meta_for_block.erase(data) ? noErr : memWZErr;
  }

  std::shared_ptr<BlockMeta> get_block_meta(void* data) const {
    return this->meta_for_block.at(data);
  }

  Handle alloc_handle(size_t size, uint16_t flags = 0) {
    auto meta = make_shared<HandleMeta>();
    meta->data = malloc(size);
    if (!meta->data) {
      this->last_error = memFullErr;
      return nullptr;
    }
    meta->size = size;
    meta->flags = flags;
    meta->memory_manager = this->weak_from_this();
    Handle handle = reinterpret_cast<Handle>(&meta->data);
    this->meta_for_handle.emplace(handle, meta);
    this->last_error = noErr;
    return handle;
  }

  void free_handle(Handle handle) {
    this->last_error = this->meta_for_handle.erase(handle) ? noErr : memWZErr;
  }

  void resize_handle(Handle handle, size_t new_size) {
    shared_ptr<HandleMeta> meta;
    try {
      meta = this->get_handle_meta(handle);
    } catch (const out_of_range&) {
      this->last_error = memWZErr;
      return;
    }

    void* new_data = realloc(meta->data, new_size);
    if (!new_data) {
      this->last_error = memFullErr;
      return;
    }
    meta->data = new_data;
    meta->size = new_size;
    this->last_error = noErr;
  }

  void check_handle_valid(Handle handle) const {
    this->last_error = this->meta_for_handle.count(handle) ? noErr : memWZErr;
  }

  std::shared_ptr<HandleMeta> get_handle_meta(Handle handle) const {
    return this->meta_for_handle.at(handle);
  }

  OSErr get_last_error() const {
    return this->last_error;
  }

  void replace_handle(Handle dest, Handle src) {
    auto src_meta_it = this->meta_for_handle.find(src);
    if (src_meta_it == this->meta_for_handle.end()) {
      throw std::out_of_range("no such handle");
    }
    auto& dest_meta = this->meta_for_handle.at(dest);
    if (dest_meta->data) {
      free(dest_meta->data);
    }
    dest_meta->data = src_meta_it->second->data;
    dest_meta->size = src_meta_it->second->size;
    dest_meta->flags = src_meta_it->second->flags;
    this->meta_for_handle.erase(src_meta_it);
  }

private:
  mutable OSErr last_error = noErr;
  std::unordered_map<void*, std::shared_ptr<BlockMeta>> meta_for_block;
  std::unordered_map<Handle, std::shared_ptr<HandleMeta>> meta_for_handle;
};

static MemoryManager memory_manager;

Handle NewHandle(Size size) {
  return reinterpret_cast<Handle>(memory_manager.alloc_handle(size));
}

Handle NewHandleClear(Size size) {
  auto handle = NewHandle(size);
  if (handle) {
    memset(*handle, 0, size);
  }
  return handle;
}

Handle NewHandleWithData(const void* data, size_t size) {
  Handle ret = NewHandle(size);
  if (ret) {
    memcpy(*ret, data, size);
  }
  return ret;
}

void DisposeHandle(Handle handle) {
  memory_manager.free_handle(handle);
}

void ReplaceHandle(Handle dest, Handle src) {
  memory_manager.replace_handle(dest, src);
}

Size GetHandleSize(Handle handle) {
  return memory_manager.get_handle_meta(handle)->size;
}

void SetHandleSize(Handle handle, Size new_size) {
  memory_manager.resize_handle(handle, new_size);
}

void HPurge(Handle handle) {
  // We never purge blocks, so we just set memWZErr if the handle is invalid
  // and otherwise ignore this call
  memory_manager.check_handle_valid(handle);
}

void HNoPurge(Handle handle) {
  // We never purge blocks, so we just set memWZErr if the handle is invalid
  // and otherwise ignore this call
  memory_manager.check_handle_valid(handle);
}

OSErr MemError() {
  return memory_manager.get_last_error();
}
