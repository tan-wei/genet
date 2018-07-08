#include "buffer.hpp"
#include "exports.hpp"

namespace {
NAN_METHOD(Buffer_address_wrap) {
  if (!info[0]->IsArrayBufferView()) {
    Nan::ThrowTypeError("First argument must be an ArrayBufferView");
    return;
  }
  auto obj = info[0].As<v8::ArrayBufferView>();
  auto buf = obj->Buffer();
  auto addr = reinterpret_cast<uint64_t>(buf->GetContents().Data());
  info.GetReturnValue().Set(Nan::New(std::to_string(addr)).ToLocalChecked());
}
} // namespace

namespace genet_node {
namespace Buffer {
void init(v8::Local<v8::Object> exports) {
  auto buffer = Nan::New<v8::Object>();
  buffer->Set(
      Nan::New("address").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(Buffer_address_wrap)->GetFunction());
  exports->Set(Nan::New("Buffer").ToLocalChecked(), buffer);
}
} // namespace Buffer
} // namespace genet_node
