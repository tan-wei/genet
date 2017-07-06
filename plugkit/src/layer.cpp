#include "layer.hpp"
#include "chunk.hpp"
#include "property.hpp"
#include "slice.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

class Layer::Private {
public:
  Private(const strns &ns);

public:
  strns ns;
  std::string summary;
  std::string error;
  std::pair<uint32_t, uint32_t> range;
  double confidence = 1.0;
  Slice payload;
  LayerConstWeakPtr parent;
  FrameConstWeakPtr frame;
  std::vector<LayerConstPtr> children;
  std::vector<ChunkConstPtr> chunks;
  std::vector<PropertyConstPtr> properties;
};

Layer::Private::Private(const strns &ns) : ns(ns) {}

Layer::Layer() : d(new Private(strns())) {}

Layer::Layer(const strns &ns) : d(new Private(ns)) {}

Layer::~Layer() {}

Layer::Layer(Layer &&layer) { this->d.reset(layer.d.release()); }

strid Layer::id() const { return d->ns.primary(); }

strns Layer::ns() const { return d->ns; }

void Layer::setNs(const strns &ns) { d->ns = ns; }

std::pair<uint32_t, uint32_t> Layer::range() const { return d->range; }

void Layer::setRange(const std::pair<uint32_t, uint32_t> &range) {
  d->range = range;
}

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

void Layer::addChunk(Chunk &&chunk) {
  addChunk(std::make_shared<Chunk>(std::move(chunk)));
}

const Slice &Layer::payload() const { return d->payload; }

void Layer::setPayload(const Slice &payload) { d->payload = payload; }

const std::vector<PropertyConstPtr> &Layer::properties() const {
  return d->properties;
}

LayerConstPtr Layer::parent() const { return d->parent.lock(); }

void Layer::setParent(const LayerConstWeakPtr &layer) { d->parent = layer; }

FrameConstPtr Layer::frame() const { return d->frame.lock(); }

void Layer::setFrame(const FrameConstWeakPtr &frame) { d->frame = frame; }

PropertyConstPtr Layer::propertyFromId(strid id) const {
  for (const auto &child : d->properties) {
    if (child->id() == id) {
      return child;
    }
  }
  return PropertyConstPtr();
}

void Layer::addProperty(const PropertyConstPtr &prop) {
  d->properties.push_back(prop);
}

void Layer::addProperty(Property &&prop) {
  addProperty(std::make_shared<Property>(std::move(prop)));
}

bool Layer::hasError() const {
  std::function<bool(const std::vector<PropertyConstPtr> &)> checkError =
      [&checkError](const std::vector<PropertyConstPtr> &properties) {
        for (const auto &prop : properties) {
          if (!prop->error().empty()) {
            return true;
          }
          if (checkError(prop->properties())) {
            return true;
          }
        }
        return false;
      };
  return checkError(d->properties);
}
}
