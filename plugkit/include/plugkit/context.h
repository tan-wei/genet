#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

#include "export.h"
#include "token.h"
#include <stddef.h>
#include <stdint.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Context Context;
typedef struct Variant Variant;
typedef struct Layer Layer;

/// Allocate a memory block in the current context.
/// @remark Currently, this function is just a wrapper for `malloc()`.
PLUGKIT_EXPORT void *Context_alloc(Context *ctx, size_t size);

/// Reallocate a memory block in the current context.
/// @remark Currently, this function is just a wrapper for `realloc()`.
PLUGKIT_EXPORT void *Context_realloc(Context *ctx, void *ptr, size_t size);

/// Deallocate a memory block in the current context.
/// @remark Currently, this function is just a wrapper for `free()`.
PLUGKIT_EXPORT void Context_dealloc(Context *ctx, void *ptr);

PLUGKIT_EXPORT Layer *Context_allocLayer(Context *ctx, Token id);
PLUGKIT_EXPORT void Context_deallocLayer(Context *ctx, Layer *layer);

/// Return the value of the option in the current context.
PLUGKIT_EXPORT const Variant *Context_getOption(Context *ctx, const char *key);

PLUGKIT_NAMESPACE_END

#endif
