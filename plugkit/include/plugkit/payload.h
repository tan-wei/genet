/// @file
/// Payload
#ifndef PLUGKIT_PAYLOAD_H
#define PLUGKIT_PAYLOAD_H

#include <cstddef>
#include "token.h"
#include "slice.h"
#include "export.h"

extern "C" {

namespace plugkit {

class Payload;
class Property;

/// Returns the payload data.
PLUGKIT_EXPORT Slice Payload_data(const Payload *payload);

/// Allocates a new Property and adds it as a payload property.
PLUGKIT_EXPORT Property *Payload_addProperty(Payload *payload, Token id);
}
}

#endif
