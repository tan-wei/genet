#include "slice.hpp"
#include "exports.hpp"

namespace {
NAN_METHOD(ByteSlice_address_wrap) {
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
namespace ByteSlice {
void init(v8::Local<v8::Object> exports) {
  auto slice = Nan::New<v8::Object>();
  slice->Set(
      Nan::New("address").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(ByteSlice_address_wrap)->GetFunction());
  exports->Set(Nan::New("ByteSlice").ToLocalChecked(), slice);
}
} // namespace ByteSlice
} // namespace genet_node
