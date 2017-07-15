#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include <memory>
#include <string>
#include <vector>
#include "export.hpp"
#include "minins.hpp"

namespace plugkit {

class Layer;
class Property;
class Frame;
class Slice;

class PLUGKIT_EXPORT Layer final {
public:
  Layer();
  Layer(const minins &ns);
  Layer(Layer &&layer);
  ~Layer();

  miniid id() const;
  minins ns() const;
  void setNs(const minins &ns);
  std::string summary() const;
  void setSummary(const std::string &summary);
  std::pair<uint32_t, uint32_t> range() const;
  void setRange(const std::pair<uint32_t, uint32_t> &range);
  double confidence() const;
  void setConfidence(double confidence);
  std::string error() const;
  void setError(const std::string &error);

  const std::vector<Layer *> &children() const;
  void addChild(Layer *child);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(miniid id) const;
  void addProperty(const Property *prop);
  void addProperty(Property &&prop);

  minins streamId() const;
  void setStreamId(const minins &id);

  const Slice &payload() const;
  void setPayload(const Slice &payload);

  const Layer *parent() const;
  void setParent(const Layer *layer);

  const Frame *frame() const;
  void setFrame(const Frame *frame);

  bool hasError() const;

private:
  Layer(const Layer &layer) = delete;
  Layer &operator=(const Layer &layer) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
