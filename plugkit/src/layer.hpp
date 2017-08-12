#ifndef PLUGKIT_LAYER_HPP
#define PLUGKIT_LAYER_HPP

#include <memory>
#include <string>
#include <vector>
#include "layer.h"
#include "token.h"
#include "types.hpp"

namespace plugkit {

struct Layer final {
public:
  Layer(Token id);
  ~Layer();

  Token id() const;
  double confidence() const;
  void setConfidence(double confidence);

  const std::vector<Layer *> &children() const;
  void addLayer(Layer *child);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(Token id) const;
  void addProperty(const Property *prop);

  uint32_t streamId() const;
  void setStreamId(uint32_t id);

  const std::vector<const Payload *> &payloads() const;
  void addPayload(const Payload *payload);

  const std::vector<Token> &tags() const;
  void addTag(Token token);

  const Layer *parent() const;
  void setParent(const Layer *layer);

  const Frame *frame() const;
  void setFrame(const Frame *frame);

private:
  Layer(const Layer &layer) = delete;
  Layer &operator=(const Layer &layer) = delete;

private:
  Token mId = 0;
  uint32_t mStreamId = 0;
  double mConfidence = 1.0;
  std::vector<const Payload *> mPayloads;
  const Layer *mParent = nullptr;
  const Frame *mFrame = nullptr;
  std::vector<Token> mTags;
  std::vector<Layer *> mChildren;
  std::vector<const Property *> mProperties;
};
}

#endif
