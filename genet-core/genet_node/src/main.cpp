#include "exports.hpp"
#include "module.hpp"
#include <nan.h>

using namespace genet_node;

namespace {
void Init(v8::Local<v8::Object> exports) { Module::init(exports); }
} // namespace

NODE_MODULE(genet_node, Init)
