#ifndef Types_CC_hpp
#define Types_CC_hpp

/**
 * This file defines types only used in C++. C++ source files should include
 * this file instead of Types.h; this file also provides everything provided by
 * Types.h.
 */

#include "Types.h"

#include <phosg/Encoding.hh>
#include <phosg/Strings.hh>
#include <stdexcept>
#include <string>

using le_uint16_t = phosg::le_uint16_t;
using le_int16_t = phosg::le_int16_t;
using le_uint32_t = phosg::le_uint32_t;
using le_int32_t = phosg::le_int32_t;
using le_uint64_t = phosg::le_uint64_t;
using le_int64_t = phosg::le_int64_t;
using le_float = phosg::le_float;
using le_double = phosg::le_double;
using be_uint16_t = phosg::be_uint16_t;
using be_int16_t = phosg::be_int16_t;
using be_uint32_t = phosg::be_uint32_t;
using be_int32_t = phosg::be_int32_t;
using be_uint64_t = phosg::be_uint64_t;
using be_int64_t = phosg::be_int64_t;
using be_float = phosg::be_float;
using be_double = phosg::be_double;

template <bool BE>
using U16T = typename std::conditional<BE, be_uint16_t, le_uint16_t>::type;
template <bool BE>
using S16T = typename std::conditional<BE, be_int16_t, le_int16_t>::type;
template <bool BE>
using U32T = typename std::conditional<BE, be_uint32_t, le_uint32_t>::type;
template <bool BE>
using S32T = typename std::conditional<BE, be_int32_t, le_int32_t>::type;
template <bool BE>
using F32T = typename std::conditional<BE, be_float, le_float>::type;

template <size_t NumChars>
std::string string_for_pstr(const unsigned char pstr[NumChars]) {
  if (pstr[0] >= NumChars) {
    throw std::runtime_error(phosg::string_printf(
        "Pascal string size field is too large; expected at most %zu bytes, received %hhu bytes",
        NumChars, pstr[0]));
  }
  return std::string(reinterpret_cast<const char*>(pstr + 1), pstr[0]);
}

template <size_t NumChars>
void pstr_for_string(unsigned char pstr[NumChars], const std::string& s) {
  if (s.size() >= NumChars) {
    throw std::runtime_error(phosg::string_printf(
        "data too long for Pascal string; expected at most %zu bytes, received %zu bytes",
        NumChars, s.size()));
  }
  pstr[0] = s.size();
  memcpy(&pstr[1], s.data(), s.size());
}

#endif // Types_CC_hpp
