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
  const Layer *parent;
  const Frame *frame;
  std::vector<const Layer *> children;
  std::vector<const Chunk *> chunks;
  std::vector<const Property *> properties;
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

const std::vector<const Layer *> &Layer::children() const {
  return d->children;
}

void Layer::addChild(const Layer *child) { d->children.push_back(child); }

const std::vector<const Chunk *> &Layer::chunks() const { return d->chunks; }

void Layer::addChunk(const Chunk *chunk) { d->chunks.push_back(chunk); }

void Layer::addChunk(Chunk &&chunk) {
  // TODO:ALLOC
  addChunk(new Chunk(std::move(chunk)));
}

const Slice &Layer::payload() const { return d->payload; }

void Layer::setPayload(const Slice &payload) { d->payload = payload; }

const std::vector<const Property *> &Layer::properties() const {
  return d->properties;
}

const Layer *Layer::parent() const { return d->parent; }

void Layer::setParent(const Layer *layer) { d->parent = layer; }

const Frame *Layer::frame() const { return d->frame; }

void Layer::setFrame(const Frame *frame) { d->frame = frame; }

const Property *Layer::propertyFromId(strid id) const {
  for (const auto &child : d->properties) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Layer::addProperty(const Property *prop) { d->properties.push_back(prop); }

void Layer::addProperty(Property &&prop) {
  // TODO:ALLOC
  addProperty(new Property(std::move(prop)));
}

bool Layer::hasError() const {
  std::function<bool(const std::vector<const Property *> &)> checkError =
      [&checkError](const std::vector<const Property *> &properties) {
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
