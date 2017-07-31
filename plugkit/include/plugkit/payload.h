/// @file
/// Payload
#ifndef PLUGKIT_PAYLOAD_H
#define PLUGKIT_PAYLOAD_H

#include "token.h"
#include <cstddef>

extern "C" {

namespace plugkit {

class Payload;
class Property;

/// Returns the payload length and assigns the first address of bytes to data.
size_t Payload_data(const Payload *layer, const char **data);

/// Sets data to the payload.
/// Payload does not take the ownership of data.
void Payload_setData(Payload *layer, const char *data, size_t length);

/// Allocates a new Property and adds it as a payload property.
Property *Payload_addProperty(Payload *layer, Token id);
}
}

#endif
