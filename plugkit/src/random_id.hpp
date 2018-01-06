#ifndef PLUGKIT_RANDOM_ID_H
#define PLUGKIT_RANDOM_ID_H

#include <random>
#include <string>

namespace plugkit {

namespace RandomID {
template <size_t length>
std::string generate() {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<char> index('a', 'z');
  std::string str(length, ' ');
  for (size_t i = 0; i < length; ++i) {
    str[i] = index(mt);
  }
  return str;
}
} // namespace RandomID

} // namespace plugkit

#endif
