#ifndef PLUGKIT_FMT_H
#define PLUGKIT_FMT_H

#include <algorithm>

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
}
}

#endif
