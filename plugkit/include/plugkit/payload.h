/// @file
/// Payload
#ifndef PLUGKIT_PAYLOAD_H
#define PLUGKIT_PAYLOAD_H

#include <stddef.h>
#include "token.h"
#include "slice.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Payload Payload;
typedef struct Attr Attr;

/// Returns the the first payload data.
PLUGKIT_EXPORT Slice Payload_slice(const Payload *payload);

PLUGKIT_EXPORT void Payload_addSlice(Payload *payload, Slice slice);

/// Returns the first address of slices
/// and assigns the number of slices to size.
PLUGKIT_EXPORT const Slice *Payload_slices(const Payload *payload,
                                           size_t *size);

/// Allocates a new Attr and adds it as a payload attribute.
PLUGKIT_EXPORT Attr *Payload_addAttr(Payload *payload, Token id);

/// Gets type
PLUGKIT_EXPORT Token Payload_type(const Payload *payload);

/// Sets type
PLUGKIT_EXPORT void Payload_setType(Payload *payload, Token type);

PLUGKIT_NAMESPACE_END

#endif
