/// @file
/// Protocol stack layer
#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include "token.h"
#include "slice.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

struct Layer;
typedef struct Layer Layer;

struct Payload;
typedef struct Payload Payload;

struct Property;
typedef struct Property Property;

/// Gets id
PLUGKIT_EXPORT Token Layer_id(const Layer *layer);

/// Gets confidence
PLUGKIT_EXPORT double Layer_confidence(const Layer *layer);

/// Sets confidence
PLUGKIT_EXPORT void Layer_setConfidence(Layer *layer, double confidence);

/// Gets parent layer
PLUGKIT_EXPORT const Layer *Layer_parent(const Layer *layer);

/// Allocates a new Layer and adds it as a child layer.
PLUGKIT_EXPORT Layer *Layer_addLayer(Layer *layer, Token id);

/// Allocates a new Property and adds it as a layer property.
PLUGKIT_EXPORT Property *Layer_addProperty(Layer *layer, Token id);

/// Finds the first layer property with the given id and returns it.
///
/// If no property is found, returns nullptr.
PLUGKIT_EXPORT const Property *Layer_propertyFromId(const Layer *layer,
                                                    Token id);

/// Allocates a new Payload and adds it as a layer payload.
PLUGKIT_EXPORT Payload *Layer_addPayload(Layer *layer, Slice slice);

/// Returns the first address of payloads
/// and assigns the number of the layer payloads to size.
PLUGKIT_EXPORT const Payload *const *Layer_payloads(const Layer *layer,
                                                    size_t *size);

/// Returns the first payload
PLUGKIT_EXPORT const Payload *Layer_payload(const Layer *layer);

/// Adds a layer tag
PLUGKIT_EXPORT void Layer_addTag(Layer *layer, Token tag);

PLUGKIT_NAMESPACE_END

#endif
