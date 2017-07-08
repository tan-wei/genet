#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include <memory>
#include <string>
#include <vector>
#include "export.hpp"
#include "strns.hpp"

namespace plugkit {

class Layer;
class Property;
class Frame;
class Chunk;
class Slice;

class PLUGKIT_EXPORT Layer final {
public:
  Layer();
  Layer(const strns &ns);
  Layer(Layer &&layer);
  ~Layer();

  strid id() const;
  strns ns() const;
  void setNs(const strns &ns);
  std::string summary() const;
  void setSummary(const std::string &summary);
  std::pair<uint32_t, uint32_t> range() const;
  void setRange(const std::pair<uint32_t, uint32_t> &range);
  double confidence() const;
  void setConfidence(double confidence);
  std::string error() const;
  void setError(const std::string &error);

  const std::vector<const Layer *> &children() const;
  void addChild(const Layer *child);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(strid id) const;
  void addProperty(const Property *prop);
  void addProperty(Property &&prop);

  const std::vector<const Chunk *> &chunks() const;
  void addChunk(const Chunk *chunk);
  void addChunk(Chunk &&chunk);

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
