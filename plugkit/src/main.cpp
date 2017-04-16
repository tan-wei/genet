#include <nan.h>
#include "session.hpp"
#include "extended_slot.hpp"
#include "plugkit_module.hpp"
#include "private/variant.hpp"

using namespace plugkit;

NAN_GC_CALLBACK(gcPrologueCallback) {
  Variant::Private::cleanupSharedBuffers();
}

void Init(v8::Local<v8::Object> exports) {
  Nan::AddGCPrologueCallback(gcPrologueCallback);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  ExtendedSlot::init(isolate);
  ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE,
                    new PlugkitModule(isolate, exports, true));
}

NODE_MODULE(plugkit, Init)
