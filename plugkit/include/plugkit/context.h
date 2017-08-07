#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

#include <cstddef>
#include "export.h"

extern "C" {

namespace plugkit {

class Variant;
class Layer;
class Context;

/// Gets options.
PLUGKIT_EXPORT const Variant *Context_options(Context *ctx);

/// Registers the stream identifier for the given layer.
///
/// @remarks This function can be used only in analyze().
PLUGKIT_EXPORT void Context_addStreamIdentifier(Context *ctx, Layer *layer,
                                                const char *str);
}
}

#endif
