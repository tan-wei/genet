#include "../src/pcap.hpp"
#include "session_context.hpp"
#include "pcap.hpp"

namespace plugkit {

void PcapWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  auto obj = Nan::New<v8::Object>();
  Nan::SetAccessor(obj, Nan::New("devices").ToLocalChecked(), devices);
  Nan::SetAccessor(obj, Nan::New("permission").ToLocalChecked(), permission);
  Nan::Set(exports, Nan::New("Pcap").ToLocalChecked(), obj);
}

PcapWrapper::PcapWrapper() {}

NAN_GETTER(PcapWrapper::devices) {
  using namespace v8;
  SessionContext ctx;
  Isolate *isolate = Isolate::GetCurrent();
  const std::vector<NetworkInterface> &devs = Pcap::create(&ctx)->devices();
  Local<Array> array = Array::New(isolate, devs.size());
  for (size_t i = 0; i < devs.size(); ++i) {
    const NetworkInterface &dev = devs[i];
    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New("id").ToLocalChecked(),
             Nan::New(dev.id).ToLocalChecked());
    obj->Set(Nan::New("name").ToLocalChecked(),
             Nan::New(dev.name).ToLocalChecked());
    obj->Set(Nan::New("description").ToLocalChecked(),
             Nan::New(dev.description).ToLocalChecked());
    obj->Set(Nan::New("link").ToLocalChecked(), Nan::New(dev.link));
    obj->Set(Nan::New("loopback").ToLocalChecked(), Nan::New(dev.loopback));
    array->Set(i, obj);
  }
  info.GetReturnValue().Set(array);
}

NAN_GETTER(PcapWrapper::permission) {
  SessionContext ctx;
  info.GetReturnValue().Set(Pcap::create(&ctx)->hasPermission());
}
} // namespace plugkit
