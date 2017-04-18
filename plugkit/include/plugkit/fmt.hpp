#ifndef PLUGKIT_FMT_H
#define PLUGKIT_FMT_H

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace plugkit {
namespace fmt {

template <class T, class S> T readLE(const S &slice, size_t offset = 0) {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  return *reinterpret_cast<const T *>(slice.data() + offset);
}

template <class T, class S> T readBE(const S &slice, size_t offset = 0) {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  char data[sizeof(T)];
  const char *begin = slice.data() + offset;
  std::reverse_copy(begin, begin + sizeof(T), data);
  return *reinterpret_cast<const T *>(data);
}

template <class S>
std::string toHex(const S &slice, int group = 0, int width = 2,
                  char sep = ':') {
  std::stringstream stream;
  stream << std::hex << std::setfill('0');
  for (size_t i = 0; i < slice.size(); ++i) {
    stream << std::setw(width) << +static_cast<uint8_t>(slice[i]);
    if (group > 0 && (i + 1) % group == 0 && i < slice.size() - 1) {
      stream << sep;
    }
  }
  return stream.str();
}

template <class S>
std::string toDec(const S &slice, int group = 0, int width = 0,
                  char sep = '.') {
  std::stringstream stream;
  stream << std::dec << std::setfill('0');
  for (size_t i = 0; i < slice.size(); ++i) {
    stream << std::setw(width) << +static_cast<uint8_t>(slice[i]);
    if (group > 0 && (i + 1) % group == 0 && i < slice.size() - 1) {
      stream << sep;
    }
  }
  return stream.str();
}
}
}

#endif
