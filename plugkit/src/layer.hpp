#ifndef PLUGKIT_LAYER_HPP
#define PLUGKIT_LAYER_HPP

#include <memory>
#include <string>
#include <vector>
#include "layer.h"
#include "token.h"
#include "error.h"
#include "types.hpp"

namespace plugkit {

struct Layer final {
public:
  Layer(Token id);
  ~Layer();

  Token id() const;
  float confidence() const;
  void setConfidence(float confidence);

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

  const std::vector<Error> &errors() const;
  void addError(Error err);

  Layer *parent() const;
  void setParent(Layer *layer);

  const Frame *frame() const;
  void setFrame(const Frame *frame);

private:
  Layer(const Layer &layer) = delete;
  Layer &operator=(const Layer &layer) = delete;

private:
  Token mId = 0;
  uint32_t mStreamId = 0;
  float mConfidence = 1.0f;
  Layer *mParent = nullptr;
  const Frame *mFrame = nullptr;
  std::vector<const Payload *> mPayloads;
  std::vector<Token> mTags;
  std::vector<Layer *> mChildren;
  std::vector<const Property *> mProperties;
  std::vector<Error> mErrors;
};
}

#endif
