#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

#include <cstddef>

extern "C" {

namespace plugkit {

class Variant;
class Layer;
class Context;

void *Context_alloc(Context *ctx, size_t size);
void Context_dealloc(Context *ctx, void *ptr);

const Variant *Context_options(Context *ctx);
void Context_addStreamIdentifier(Context *ctx, Layer *layer, const char *str);
}
}

#endif
