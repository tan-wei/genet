/// @file
/// Payload
#ifndef PLUGKIT_PAYLOAD_H
#define PLUGKIT_PAYLOAD_H

#include "token.h"
#include "view.h"
#include <cstddef>

extern "C" {

namespace plugkit {

class Payload;
class Property;

/// Returns the payload data.
View Payload_data(const Payload *payload);

/// Allocates a new Property and adds it as a payload property.
Property *Payload_addProperty(Payload *payload, Token id);
}
}

#endif
