#pragma once

#include "MemoryManager.h"

#include <functional>
#include <phosg/Strings.hh>
#include <string>

inline Handle NewHandleWithData(const std::string& data) {
  return NewHandleWithData(data.data(), data.size());
}

void replace_handle_data(Handle handle, const void* data, size_t size);

inline std::string string_for_handle(Handle handle) {
  return std::string(reinterpret_cast<const char*>(*handle), GetHandleSize(handle));
}

inline phosg::StringReader read_from_handle(Handle handle) {
  return phosg::StringReader(*handle, GetHandleSize(handle));
}

// NewHandleTyped and DisposeHandleTyped are used for allocating C++ objects
// using the Memory Manager interface. If ObjT's destructor is nontrivial, it
// must be destroyed using DisposeHandleTyped; if it's trivial, it may be
// destroyed using the standard DisposeHandle function.

template <typename ObjT, typename... ArgTs>
ObjT** NewHandleTyped(ArgTs&&... args) {
  ObjT** ret = reinterpret_cast<ObjT**>(NewHandle(sizeof(ObjT)));
  new (*ret) ObjT(std::forward<ArgTs>(args)...); // Call constructor
  return ret;
}

template <typename ObjT>
void DisposeHandleTyped(ObjT** handle) {
  (*handle)->~ObjT();
  DisposeHandle(reinterpret_cast<Handle>(handle));
}

void add_destroy_callback(Handle handle, std::function<void()> cb);
