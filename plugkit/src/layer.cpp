#include "layer.hpp"
#include "wrapper/layer.hpp"
#include "slice.hpp"
#include "property.hpp"
#include <unordered_map>

namespace plugkit {

class Layer::Private {
public:
  Private(const std::string &ns, const std::string &name);

public:
  std::string ns;
  std::string name;
  std::string id;
  std::string summary;
  std::string error;
  std::string range = ":";
  double confidence = 1.0;
  Slice payload;
  LayerConstWeakPtr parent;
  std::vector<LayerConstPtr> children;
  std::vector<ChunkConstPtr> chunks;
  std::vector<PropertyConstPtr> properties;
  std::unordered_map<std::string, size_t> idMap;
};

Layer::Private::Private(const std::string &ns, const std::string &name)
    : ns(ns), name(name) {}

Layer::Layer() : d(new Private("", "")) {}

Layer::Layer(const std::string &ns, const std::string &name)
    : d(new Private(ns, name)) {}

Layer::~Layer() {}

Layer::Layer(Layer &&layer) { this->d.reset(layer.d.release()); }

std::string Layer::ns() const { return d->ns; }

void Layer::setNs(const std::string &ns) { d->ns = ns; }

std::string Layer::name() const { return d->name; }

void Layer::setName(const std::string &name) { d->name = name; }

std::string Layer::range() const { return d->range; }

void Layer::setRange(const std::string &range) { d->range = range; }

std::string Layer::summary() const { return d->summary; }

void Layer::setSummary(const std::string &summary) { d->summary = summary; }

double Layer::confidence() const { return d->confidence; }

void Layer::setConfidence(double confidence) { d->confidence = confidence; }

std::string Layer::error() const { return d->error; }

void Layer::setError(const std::string &error) { d->error = error; }

const std::vector<LayerConstPtr> &Layer::children() const {
  return d->children;
}

void Layer::addChild(const LayerPtr &child) { d->children.push_back(child); }

const std::vector<ChunkConstPtr> &Layer::chunks() const { return d->chunks; }

void Layer::addChunk(const ChunkConstPtr &chunk) { d->chunks.push_back(chunk); }

const Slice &Layer::payload() const { return d->payload; }

void Layer::setPayload(const Slice &payload) { d->payload = payload; }

const std::vector<PropertyConstPtr> &Layer::properties() const {
  return d->properties;
}

LayerConstPtr Layer::parent() const { return d->parent.lock(); }

void Layer::setParent(const LayerConstWeakPtr &layer) { d->parent = layer; }

PropertyConstPtr Layer::propertyFromId(const std::string &id) const {
  auto it = d->idMap.find(id);
  if (it != d->idMap.end()) {
    return d->properties[it->second];
  } else {
    return PropertyConstPtr();
  }
}

void Layer::addProperty(const PropertyConstPtr &prop) {
  d->idMap[prop->id()] = d->properties.size();
  d->properties.push_back(prop);
}

void Layer::addProperty(Property &&prop) {
  addProperty(std::make_shared<Property>(std::move(prop)));
}
}
