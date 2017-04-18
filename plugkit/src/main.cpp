#include <nan.h>
#include "session.hpp"
#include "extended_slot.hpp"
#include "plugkit_module.hpp"
#include "private/variant.hpp"

#if defined(PLUGKIT_OS_LINUX)
#include <dlfcn.h>
#endif

using namespace plugkit;

NAN_GC_CALLBACK(gcPrologueCallback) {
  Variant::Private::cleanupSharedBuffers();
}

void Init(v8::Local<v8::Object> exports) {
#if defined(PLUGKIT_OS_LINUX)
  Dl_info info;
  if (dladdr(reinterpret_cast<const void *>(&Init), &info) != 0) {
    dlopen(info.dli_fname, RTLD_LAZY | RTLD_NOLOAD | RTLD_GLOBAL);
  }
#endif

  Nan::AddGCPrologueCallback(gcPrologueCallback);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  ExtendedSlot::init(isolate);
  ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE,
                    new PlugkitModule(isolate, exports, true));
}

NODE_MODULE(plugkit, Init)
