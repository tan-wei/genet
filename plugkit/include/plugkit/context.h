#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

#include <stddef.h>
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

struct Variant;
typedef struct Variant Variant;

struct Layer;
typedef struct Layer Layer;

struct Context;
typedef struct Context Context;

/// Gets options.
PLUGKIT_EXPORT const Variant *Context_options(Context *ctx);

/// Registers the stream identifier for the given layer.
///
/// @remarks This function can be used only in analyze().
PLUGKIT_EXPORT void Context_addStreamIdentifier(Context *ctx, Layer *layer,
                                                const char *str);

PLUGKIT_NAMESPACE_END

#endif
