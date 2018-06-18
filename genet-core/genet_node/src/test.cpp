#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <nan.h>

#include "exports.hpp"
#include "module.hpp"

using namespace genet_node;

namespace {
void Init(v8::Local<v8::Object> exports) {
  Module::init(exports);
  int result = Catch::Session().run();
  if (result != 0) {
    exit(result);
  }
  Module::destroy();
}
} // namespace

NODE_MODULE(genet_node_test, Init)
