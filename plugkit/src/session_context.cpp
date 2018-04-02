#include "session_context.hpp"
#include <cstdio>

namespace plugkit {
extern "C" {
SharedContext *plugkit_in_create_shared_ctx();
void plugkit_in_destroy_shared_ctx(SharedContext *ctx);
}

SessionContext::SessionContext() : shared(plugkit_in_create_shared_ctx()) {
  printf("@@@ %p @@@", shared);
}

SessionContext::~SessionContext() { plugkit_in_destroy_shared_ctx(shared); }

} // namespace plugkit