#ifndef PLUGKIT_PCAP_WRAPPER_H
#define PLUGKIT_PCAP_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class PcapWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static NAN_GETTER(devices);
  static NAN_GETTER(permission);

private:
  PcapWrapper();
  PcapWrapper(const PcapWrapper &) = delete;
  PcapWrapper &operator=(const PcapWrapper &) = delete;
};
} // namespace plugkit

#endif
