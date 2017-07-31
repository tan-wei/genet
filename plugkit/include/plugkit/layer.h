/// @file
/// Protocol stack layer
#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include "token.h"

extern "C" {

namespace plugkit {

class Layer;
class Frame;
class Property;

/// Gets id
Token Layer_id(const Layer *layer);

/// Gets confidence
double Layer_confidence(const Layer *layer);

/// Sets confidence
void Layer_setConfidence(Layer *layer, double confidence);

/// Gets parent layer
const Layer *Layer_parent(const Layer *layer);

/// Gets frame
const Frame *Layer_frame(const Layer *layer);

/// Allocates a new Layer and adds it as a child layer.
///
/// @return Added Layer
Layer *Layer_addLayer(Layer *layer);

/// Returns the first address of an array of the child layers.
///
/// The array ends with nullptr.
const Layer **Layer_layers(const Layer *layer);

/// Allocates a new Property and adds it as a layer property.
///
/// @return Added Property
Property *Layer_addProperty(Layer *layer);

/// Finds the first layer property with the given id and returns it.
///
/// If no property is found, returns nullptr.
///
/// @return Found Property
const Property *Layer_propertyFromId(const Layer *layer, Token id);

/// Returns the first address of an array of the layer properties.
///
/// The array ends with nullptr.
const Property **Layer_properties(const Layer *layer);

/// Adds a layer tag
void Layer_addTag(Layer *layer, Token tag);
}
}

#endif
