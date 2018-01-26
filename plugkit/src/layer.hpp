#ifndef PLUGKIT_LAYER_HPP
#define PLUGKIT_LAYER_HPP

#include "layer.h"
#include "range.h"
#include "token.h"
#include "types.hpp"
#include <memory>
#include <string>
#include <vector>

namespace plugkit {

struct Context;

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
  uint8_t mWorker : 4;
  LayerConfidence mConfidence : 2;
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
} // namespace plugkit

#endif
