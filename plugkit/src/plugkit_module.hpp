#ifndef PLUGKIT_PLUGKIT_MODULE_HPP
#define PLUGKIT_PLUGKIT_MODULE_HPP

#include <v8.h>
#include <vector>

namespace plugkit {

class PlugkitModule final {
public:
  PlugkitModule(v8::Isolate *isolate, v8::Local<v8::Object> exports,
                bool mainThread);
  PlugkitModule(const PlugkitModule &) = delete;
  PlugkitModule &operator=(const PlugkitModule &) = delete;
  static PlugkitModule *get(v8::Isolate *isolate);

public:
  using Function = v8::UniquePersistent<v8::Function>;
  using Prototype = v8::UniquePersistent<v8::Value>;
  struct Class {
    Function ctor;
    Prototype proto;
  };

  Class property;
  Class layer;
  Class frame;
  Class session;
  Class sessionFactory;
  Class dissectorFactory;
  Class streamDissectorFactory;

  Function arrayToBuffer;
};
}

#endif
