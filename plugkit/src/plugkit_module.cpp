#include "plugkit_module.hpp"
#include "extended_slot.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/pcap.hpp"
#include "wrapper/payload.hpp"
#include "wrapper/property.hpp"
#include "wrapper/session.hpp"
#include "wrapper/session_factory.hpp"
#include "private/variant.hpp"

namespace plugkit {

PlugkitModule::PlugkitModule(v8::Isolate *isolate,
                             v8::Local<v8::Object> exports, bool mainThread) {
  ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE, this);
  Variant::Private::init(isolate);
  PropertyWrapper::init(isolate, exports);
  LayerWrapper::init(isolate, exports);
  FrameWrapper::init(isolate);
  PayloadWrapper::init(isolate);
  if (mainThread) {
    PcapWrapper::init(isolate, exports);
    SessionFactoryWrapper::init(isolate, exports);
    SessionWrapper::init(isolate, exports);
  }
}

PlugkitModule *PlugkitModule::get(v8::Isolate *isolate) {
  return ExtendedSlot::get<PlugkitModule>(isolate,
                                          ExtendedSlot::SLOT_PLUGKIT_MODULE);
}
}
