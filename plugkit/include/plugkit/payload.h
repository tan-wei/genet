/// @file
/// Payload
#ifndef PLUGKIT_PAYLOAD_H
#define PLUGKIT_PAYLOAD_H

#include "export.h"
#include "range.h"
#include "slice.h"
#include "token.h"
#include <stddef.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Context Context;
typedef struct Payload Payload;
typedef struct Attr Attr;

PLUGKIT_EXPORT void Payload_addSlice(Payload *payload, Slice slice);

/// Returns the first address of slices
/// and assigns the number of slices to size.
/// Returns the address of an empty slice if the payload has no slices.
PLUGKIT_EXPORT const Slice *Payload_slices(const Payload *payload,
                                           size_t *size);

/// Allocates a new Attr and adds it as a payload attribute.
PLUGKIT_EXPORT Attr *Payload_addAttr(Context *ctx, Payload *payload, Token id);

/// Return the type of the payload
PLUGKIT_EXPORT Token Payload_type(const Payload *payload);

/// Set the type of the payload
PLUGKIT_EXPORT void Payload_setType(Payload *payload, Token type);

/// Return the range of the payload.
PLUGKIT_EXPORT Range Payload_range(const Payload *payload);

/// Set the range of the payload.
PLUGKIT_EXPORT void Payload_setRange(Payload *payload, Range range);

PLUGKIT_NAMESPACE_END

#endif
