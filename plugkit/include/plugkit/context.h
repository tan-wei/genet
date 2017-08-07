#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

#include <cstddef>

extern "C" {

namespace plugkit {

class Variant;
class Layer;
class Context;

/// Gets options.
const Variant *Context_options(Context *ctx);

/// Registers the stream identifier for the given layer.
///
/// @remarks This function can be used only in analyze().
void Context_addStreamIdentifier(Context *ctx, Layer *layer, const char *str);
}
}

#endif
