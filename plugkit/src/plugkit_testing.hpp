#ifndef PLUGKIT_PLUGKIT_TESTING_HPP
#define PLUGKIT_PLUGKIT_TESTING_HPP

#include <v8.h>

namespace plugkit {

class PlugkitTesting final {
public:
  PlugkitTesting() = delete;
  PlugkitTesting(const PlugkitTesting &) = delete;
  PlugkitTesting &operator=(const PlugkitTesting &) = delete;
  static void init(v8::Local<v8::Object> exports);
};
} // namespace plugkit

#endif
