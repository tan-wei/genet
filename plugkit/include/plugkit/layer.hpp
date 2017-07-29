#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include <memory>
#include <string>
#include <vector>
#include "export.hpp"
#include "minins.hpp"
#include "types.hpp"

namespace plugkit {

class PLUGKIT_EXPORT Layer final {
public:
  Layer(const minins &ns);
  ~Layer();

  miniid id() const;
  minins ns() const;
  void setNs(const minins &ns);
  std::string summary() const;
  void setSummary(const std::string &summary);
  double confidence() const;
  void setConfidence(double confidence);

  const std::vector<Layer *> &children() const;
  void addChild(Layer *child);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(miniid id) const;
  void addProperty(const Property *prop);

  uint32_t streamId() const;
  void setStreamId(uint32_t id);

  const Slice &payload() const;
  void setPayload(const Slice &payload);

  const std::vector<const Payload *> &payloads() const;
  void addPayload(const Payload *payload);

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
