#include "version.hpp"
#include "exports.hpp"

namespace genet_node {
namespace Version {
void init(v8::Local<v8::Object> exports) {
  auto version = Nan::New<v8::Object>();

  uint64_t abiVersion = genet_abi_version();
  version->Set(Nan::New("abi").ToLocalChecked(),
               Nan::New(std::to_string(abiVersion >> 32) + "." +
                        std::to_string(abiVersion & 0xffffffff))
                   .ToLocalChecked());

  exports->Set(Nan::New("version").ToLocalChecked(), version);
}
} // namespace Version
} // namespace genet_node
