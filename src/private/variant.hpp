#ifndef PLUGKIT_VARIANT_PRIVATE_H
#define PLUGKIT_VARIANT_PRIVATE_H

#include "../plugkit/variant.hpp"
#include <json11.hpp>
#include <v8.h>

namespace plugkit {

class Variant::Private {
public:
  static v8::Local<v8::Object> getNodeBuffer(const Slice &slice);
  static Slice getSlice(v8::Local<v8::Object> obj);
  static v8::Local<v8::Value> getValue(const Variant &var);
  static Variant getVariant(v8::Local<v8::Value> var);
  static json11::Json getJson(const Variant &var);
  static void cleanupSharedBuffers();
};
}

#endif
