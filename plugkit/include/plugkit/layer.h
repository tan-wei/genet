/// @file
/// Protocol stack layer
#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include "export.h"
#include "slice.h"
#include "token.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Context Context;
typedef struct Layer Layer;
typedef struct Payload Payload;
typedef struct Attr Attr;

typedef enum {
  LAYER_CONF_DECODABLE = 0,
  LAYER_CONF_POSSIBLE = 1,
  LAYER_CONF_PROBABLE = 2,
  LAYER_CONF_EXACT = 3
} LayerConfidence;

/// Return the ID of the layer.
PLUGKIT_EXPORT Token Layer_id(const Layer *layer);

/// Return the confidence of the layer.
PLUGKIT_EXPORT LayerConfidence Layer_confidence(const Layer *layer);

/// Set the confidence of the layer.
PLUGKIT_EXPORT void Layer_setConfidence(Layer *layer,
                                        LayerConfidence confidence);

/// Return the worker ID of the layer.
PLUGKIT_EXPORT uint8_t Layer_worker(const Layer *layer);

/// Set the worker ID of the layer.
PLUGKIT_EXPORT void Layer_setWorker(Layer *layer, uint8_t id);

/// Return the parent of the layer.
/// If this is a root layer, return `NULL`.
PLUGKIT_EXPORT const Layer *Layer_parent(const Layer *layer);

/// Allocate a new `Layer` and adds it as a child layer.
PLUGKIT_EXPORT Layer *Layer_addLayer(Context *ctx, Layer *layer, Token id);

/// Allocate a new `Layer` and adds it as a sub layer.
PLUGKIT_EXPORT Layer *Layer_addSubLayer(Context *ctx, Layer *layer, Token id);

/// Allocate a new `Attr` and adds it as a layer attribute.
PLUGKIT_EXPORT Attr *Layer_addAttr(Context *ctx, Layer *layer, Token id);

/// Find the first layer attribute with the given id and returns it.
///
/// If no attribute is found, returns nullptr.
PLUGKIT_EXPORT const Attr *Layer_attr(const Layer *layer, Token id);

/// Allocate a new Payload and adds it as a layer payload.
PLUGKIT_EXPORT Payload *Layer_addPayload(Context *ctx, Layer *layer);

/// Return the first address of payloads
/// and assigns the number of the layer payloads to size.
/// Returns the address of an empty payload if the layer has no payloads.
PLUGKIT_EXPORT const Payload *const *Layer_payloads(const Layer *layer,
                                                    size_t *size);

/// Add a layer tag
PLUGKIT_EXPORT void Layer_addTag(Layer *layer, Token tag);

PLUGKIT_NAMESPACE_END

#endif
