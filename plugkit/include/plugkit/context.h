#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

#include <cstddef>

extern "C" {

namespace plugkit {
class Variant;

struct Context;

void *Context_alloc(Context *ctx, size_t size);
void Context_dealloc(Context *ctx, void *ptr);

const Variant *Context_options(Context *ctx);
}
}

#endif
