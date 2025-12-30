#pragma once

#include <phosg/Strings.hh>
#include <stdexcept>
#include <string>

template <size_t NumChars>
std::string string_for_pstr(const char pstr[NumChars]) {
  unsigned char size = static_cast<unsigned char>(pstr[0]);
  if (size >= NumChars) {
    throw std::runtime_error(std::format(
        "Pascal string size field is too large; expected at most {} bytes, received {} bytes",
        NumChars, size));
  }
  return std::string(reinterpret_cast<const char*>(pstr + 1), size);
}

template <size_t NumChars>
void pstr_for_string(char pstr[NumChars], const std::string& s) {
  if (s.size() >= NumChars) {
    throw std::runtime_error(std::format(
        "data too long for Pascal string; expected at most {} bytes, received {} bytes",
        NumChars, s.size()));
  }
  pstr[0] = s.size();
  memcpy(&pstr[1], s.data(), s.size());
}
