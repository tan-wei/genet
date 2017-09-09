#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

#include <stddef.h>
#include <stdint.h>
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Variant Variant;
typedef struct Layer Layer;
typedef struct Context Context;
typedef struct Slice Slice;

PLUGKIT_EXPORT void *Context_alloc(Context *ctx, size_t size);

PLUGKIT_EXPORT void Context_dealloc(Context *ctx, void *ptr);

/// Gets options.
PLUGKIT_EXPORT const Variant *Context_options(Context *ctx);

PLUGKIT_NAMESPACE_END

#endif
