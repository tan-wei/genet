#ifndef PLUGKIT_LAYER_H
#define PLUGKIT_LAYER_H

#include <memory>
#include <string>
#include <vector>

namespace plugkit {

class Layer;
using LayerPtr = std::shared_ptr<Layer>;
using LayerConstPtr = std::shared_ptr<const Layer>;
using LayerConstWeakPtr = std::weak_ptr<const Layer>;

class Property;
using PropertyConstPtr = std::shared_ptr<const Property>;

class Frame;
using FrameConstPtr = std::shared_ptr<const Frame>;
using FrameConstWeakPtr = std::weak_ptr<const Frame>;

class Chunk;
using ChunkConstPtr = std::shared_ptr<const Chunk>;

class Slice;

class Layer final {
public:
  Layer();
  Layer(const std::string &ns, const std::string &name);
  Layer(Layer &&layer);
  ~Layer();

  std::string id() const;
  std::string ns() const;
  void setNs(const std::string &ns);
  std::string name() const;
  void setName(const std::string &name);
  std::string summary() const;
  void setSummary(const std::string &summary);
  std::pair<uint32_t, uint32_t> range() const;
  void setRange(const std::pair<uint32_t, uint32_t> &range);
  double confidence() const;
  void setConfidence(double confidence);
  std::string error() const;
  void setError(const std::string &error);

  const std::vector<LayerConstPtr> &children() const;
  void addChild(const LayerPtr &child);

  const std::vector<PropertyConstPtr> &properties() const;
  PropertyConstPtr propertyFromId(const std::string &id) const;
  void addProperty(const PropertyConstPtr &prop);
  void addProperty(Property &&prop);

  const std::vector<ChunkConstPtr> &chunks() const;
  void addChunk(const ChunkConstPtr &chunk);
  void addChunk(Chunk &&chunk);

  const Slice &payload() const;
  void setPayload(const Slice &payload);

  LayerConstPtr parent() const;
  void setParent(const LayerConstWeakPtr &layer);

  FrameConstPtr frame() const;
  void setFrame(const FrameConstWeakPtr &frame);

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
