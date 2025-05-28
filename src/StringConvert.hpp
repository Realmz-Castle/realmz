#pragma once

#include <phosg/Strings.hh>
#include <stdexcept>
#include <string>

template <size_t NumChars>
std::string string_for_pstr(const unsigned char pstr[NumChars]) {
  if (pstr[0] >= NumChars) {
    throw std::runtime_error(std::format(
        "Pascal string size field is too large; expected at most {} bytes, received {} bytes",
        NumChars, pstr[0]));
  }
  return std::string(reinterpret_cast<const char*>(pstr + 1), pstr[0]);
}

template <size_t NumChars>
void pstr_for_string(unsigned char pstr[NumChars], const std::string& s) {
  if (s.size() >= NumChars) {
    throw std::runtime_error(std::format(
        "data too long for Pascal string; expected at most {} bytes, received {} bytes",
        NumChars, s.size()));
  }
  pstr[0] = s.size();
  memcpy(&pstr[1], s.data(), s.size());
}
