#ifndef PLUGKIT_LAYER_HPP
#define PLUGKIT_LAYER_HPP

#include "range.hpp"
#include "token.hpp"
#include "types.hpp"
#include <memory>
#include <string>
#include <vector>

#define LAYER_MAX_WORKER 16

namespace plugkit {

struct Context;

enum LayerConfidence {
  LAYER_CONF_ERROR = 0,
  LAYER_CONF_POSSIBLE = 1,
  LAYER_CONF_PROBABLE = 2,
  LAYER_CONF_EXACT = 3
};

struct Layer final {
public:
  Layer(Token id);
  ~Layer();

  Token id() const;

  LayerConfidence confidence() const;
  void setConfidence(LayerConfidence confidence);

  Range range() const;
  void setRange(const Range &range);

  const std::vector<Layer *> &layers() const;
  void addLayer(Layer *child);

  const std::vector<Layer *> &subLayers() const;
  void addSubLayer(Layer *child);

  const std::vector<Attr *> &attrs() const;
  const Attr *attr(Token id) const;
  void addAttr(Attr *attr);

  uint8_t worker() const;
  void setWorker(uint8_t id);

  const std::vector<Payload *> &payloads() const;
  void addPayload(Payload *payload);

  const std::vector<Token> &tags() const;
  void addTag(Token token);

  Layer *parent() const;
  void setParent(Layer *layer);

  const Layer *master() const;
  void setMaster(const Layer *layer);

  const Frame *frame() const;
  void setFrame(const Frame *frame);

  void removeUnconfidentLayers(Context *ctx, LayerConfidence confidence);

private:
  Layer(const Layer &layer) = delete;
  Layer &operator=(const Layer &layer) = delete;

private:
  Token mId = 0;
  uint8_t mData = 0;
  Layer *mParent = nullptr;
  const Layer *mMaster = nullptr;
  const Frame *mFrame = nullptr;
  Range mRange = {0, 0};
  std::vector<Payload *> mPayloads;
  std::vector<Token> mTags;
  std::vector<Layer *> mLayers;
  std::vector<Layer *> mSubLayers;
  std::vector<Attr *> mAttrs;
};

extern "C" {

/// Return the ID of the layer.
Token Layer_id(const Layer *layer);

/// Return the confidence of the layer.
LayerConfidence Layer_confidence(const Layer *layer);

/// Return the range of the layer.
Range Layer_range(const Layer *layer);

/// Set the range of the layer.
void Layer_setRange(Layer *layer, Range range);

/// Set the confidence of the layer.
void Layer_setConfidence(Layer *layer, LayerConfidence confidence);

/// Return the worker ID of the layer.
uint8_t Layer_worker(const Layer *layer);

/// Set the worker ID of the layer.
void Layer_setWorker(Layer *layer, uint8_t id);

/// Return the parent of the layer.
/// If this is a root layer, return `NULL`.
const Layer *Layer_parent(const Layer *layer);

/// Allocate a new `Layer` and adds it as a child layer.
Layer *Layer_addLayer(Layer *layer, Context *ctx, Token id);

/// Allocate a new `Layer` and adds it as a sub layer.
Layer *Layer_addSubLayer(Layer *layer, Context *ctx, Token id);

/// Allocate a new `Attr` and adds it as a layer attribute.
Attr *Layer_addAttr(Layer *layer, Context *ctx, Token id);

/// Find the first layer attribute with the given id and returns it.
///
/// If no attribute is found, returns nullptr.
const Attr *Layer_attr(const Layer *layer, Token id);

/// Allocate a new Payload and adds it as a layer payload.
Payload *Layer_addPayload(Layer *layer, Context *ctx);

/// Return the first address of payloads
/// and assigns the number of the layer payloads to size.
/// Returns the address of an empty payload if the layer has no payloads.
const Payload *const *Layer_payloads(const Layer *layer, size_t *size);

/// Add a layer tag
void Layer_addTag(Layer *layer, Token tag);
}

} // namespace plugkit

#endif
