#include "dyr_kernel.hpp"
#include "module.hpp"
#include <nan.h>

using namespace dyr_node;

namespace {
void Init(v8::Local<v8::Object> exports) {
  Module::init(exports);
}
}  // namespace

NODE_MODULE(dyr_node, Init)
