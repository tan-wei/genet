#include <node_api.h>

extern "C" {
napi_value genet_napi_init(napi_env, napi_value);
}

napi_value Init(napi_env env, napi_value exports) {
  return genet_napi_init(env, exports);
}

NAPI_MODULE(genet, Init);