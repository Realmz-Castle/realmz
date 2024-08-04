#pragma once

#include "MemoryManager.h"

#include <phosg/Strings.hh>
#include <string>

inline Handle NewHandleWithData(const std::string& data) {
  return NewHandleWithData(data.data(), data.size());
}

inline std::string string_for_handle(Handle handle) {
  return std::string(reinterpret_cast<const char*>(*handle), GetHandleSize(handle));
}

inline phosg::StringReader read_from_handle(Handle handle) {
  return phosg::StringReader(*handle, GetHandleSize(handle));
}

template <typename T>
T** NewHandleTyped(size_t size = sizeof(T)) {
  return reinterpret_cast<T**>(NewHandle(size));
}
