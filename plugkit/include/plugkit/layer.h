/// @file
/// Protocol stack layer
#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include "token.h"
#include "slice.h"

extern "C" {

namespace plugkit {

class Layer;
class Payload;
class Property;

/// Gets id
Token Layer_id(const Layer *layer);

/// Gets confidence
double Layer_confidence(const Layer *layer);

/// Sets confidence
void Layer_setConfidence(Layer *layer, double confidence);

/// Gets parent layer
const Layer *Layer_parent(const Layer *layer);

/// Allocates a new Layer and adds it as a child layer.
Layer *Layer_addLayer(Layer *layer, Token id);

/// Allocates a new Property and adds it as a layer property.
Property *Layer_addProperty(Layer *layer, Token id);

/// Finds the first layer property with the given id and returns it.
///
/// If no property is found, returns nullptr.
const Property *Layer_propertyFromId(const Layer *layer, Token id);

/// Allocates a new Payload and adds it as a layer payload.
Payload *Layer_addPayload(Layer *layer, Slice slice);

/// Returns the first address of payloads
/// and assigns the number of the layer payloads to size.
const Payload *const *Layer_payloads(const Layer *layer, size_t *size);

/// Adds a layer tag
void Layer_addTag(Layer *layer, Token tag);
}
}

#endif
