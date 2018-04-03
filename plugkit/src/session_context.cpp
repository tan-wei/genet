#include "session_context.hpp"
#include <cstdio>

namespace plugkit {
extern "C" {
SharedContextWrapper *plugkit_in_create_shared_ctx();
void plugkit_in_destroy_shared_ctx(SharedContextWrapper *ctx);
}

SessionContext::SessionContext() : shared(plugkit_in_create_shared_ctx()) {}

SessionContext::~SessionContext() { plugkit_in_destroy_shared_ctx(shared); }

} // namespace plugkit