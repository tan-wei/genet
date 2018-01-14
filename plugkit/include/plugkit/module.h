#ifndef PLUGKIT_MODULE_H
#define PLUGKIT_MODULE_H

#include <node.h>

#define PLUGKIT_MODULE(name, func)                                             \
  NODE_MODULE(plugkitModule_, [](v8::Local<v8::Object> exports,                \
                                 v8::Local<v8::Value>, void *) {               \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                          \
    exports->Set(v8::String::NewFromUtf8(isolate, name),                       \
                 v8::External::New(isolate, func()));                          \
  });

#endif
