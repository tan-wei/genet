#ifndef PLUGKIT_LAYER_HPP
#define PLUGKIT_LAYER_HPP

#include <memory>
#include <string>
#include <vector>
#include "layer.h"
#include "export.hpp"
#include "types.hpp"
#include "token.h"

namespace plugkit {

class PLUGKIT_EXPORT Layer final {
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

  const Slice &payload() const;
  void setPayload(const Slice &payload);

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
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
