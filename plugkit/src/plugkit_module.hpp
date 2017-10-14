#ifndef PLUGKIT_PLUGKIT_MODULE_HPP
#define PLUGKIT_PLUGKIT_MODULE_HPP

#include <v8.h>

namespace plugkit {

class PlugkitModule final {
public:
  PlugkitModule(v8::Isolate *isolate,
                v8::Local<v8::Object> exports,
                bool mainThread);
  PlugkitModule(const PlugkitModule &) = delete;
  PlugkitModule &operator=(const PlugkitModule &) = delete;
  static PlugkitModule *get(v8::Isolate *isolate);

public:
  using FunctionTemplate = v8::UniquePersistent<v8::FunctionTemplate>;
  using Function = v8::UniquePersistent<v8::Function>;
  using Prototype = v8::UniquePersistent<v8::Value>;
  struct Class {
    FunctionTemplate temp;
    Function ctor;
    Prototype proto;
  };

  Class attribute;
  Class context;
  Class layer;
  Class frame;
  Class payload;
  Class logger;
  Class error;
  Class session;
  Class sessionFactory;
  Class worker;

  Function arrayToBuffer;
};
} // namespace plugkit

#endif
