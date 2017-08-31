/// @file
/// Protocol stack layer
#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include "token.h"
#include "slice.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Layer Layer;
typedef struct Payload Payload;
typedef struct Attr Attr;

/// Gets id
PLUGKIT_EXPORT Token Layer_id(const Layer *layer);

/// Gets confidence
PLUGKIT_EXPORT float Layer_confidence(const Layer *layer);

/// Sets confidence
PLUGKIT_EXPORT void Layer_setConfidence(Layer *layer, float confidence);

/// Gets parent layer
PLUGKIT_EXPORT const Layer *Layer_parent(const Layer *layer);

/// Allocates a new Layer and adds it as a child layer.
PLUGKIT_EXPORT Layer *Layer_addLayer(Layer *layer, Token id);

/// Allocates a new Layer and adds it as a sub layer.
PLUGKIT_EXPORT Layer *Layer_addSubLayer(Layer *layer, Token id);

/// Allocates a new Attr and adds it as a layer attribute.
PLUGKIT_EXPORT Attr *Layer_addAttr(Layer *layer, Token id);

/// Finds the first layer attribute with the given id and returns it.
///
/// If no attribute is found, returns nullptr.
PLUGKIT_EXPORT const Attr *Layer_attr(const Layer *layer, Token id);

/// Allocates a new Payload and adds it as a layer payload.
PLUGKIT_EXPORT Payload *Layer_addPayload(Layer *layer);

/// Returns the first address of payloads
/// and assigns the number of the layer payloads to size.
PLUGKIT_EXPORT const Payload *const *Layer_payloads(const Layer *layer,
                                                    size_t *size);

/// Returns the first payload
PLUGKIT_EXPORT const Payload *Layer_payload(const Layer *layer);

/// Adds a layer tag
PLUGKIT_EXPORT void Layer_addTag(Layer *layer, Token tag);

/// Adds a layer error
PLUGKIT_EXPORT void Layer_addError(Layer *layer, Error err);

PLUGKIT_NAMESPACE_END

#endif
