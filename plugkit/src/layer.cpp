#include "layer.hpp"
#include "property.hpp"
#include "slice.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

class Layer::Private {
public:
  Private(Token id);

public:
  Token id = 0;
  uint32_t streamId = 0;
  std::string summary;
  double confidence = 1.0;
  Slice payload;
  const Layer *parent = nullptr;
  const Frame *frame = nullptr;
  std::vector<Token> tags;
  std::vector<Layer *> children;
  std::vector<const Property *> properties;
};

Layer::Private::Private(Token id) : id(id) {}

Layer::Layer(Token id) : d(new Private(id)) {}

Layer::~Layer() {}

Token Layer::id() const { return d->id; }

std::string Layer::summary() const { return d->summary; }

void Layer::setSummary(const std::string &summary) { d->summary = summary; }

double Layer::confidence() const { return d->confidence; }

void Layer::setConfidence(double confidence) { d->confidence = confidence; }

const std::vector<Layer *> &Layer::children() const { return d->children; }

void Layer::addChild(Layer *child) { d->children.push_back(child); }

uint32_t Layer::streamId() const { return d->streamId; }

void Layer::setStreamId(uint32_t id) { d->streamId = id; }

const Slice &Layer::payload() const { return d->payload; }

const std::vector<Token> &Layer::tags() const { return d->tags; }

void Layer::addTag(Token token) { d->tags.push_back(token); }

void Layer::setPayload(const Slice &payload) { d->payload = payload; }

const std::vector<const Property *> &Layer::properties() const {
  return d->properties;
}

const Layer *Layer::parent() const { return d->parent; }

void Layer::setParent(const Layer *layer) { d->parent = layer; }

const Frame *Layer::frame() const { return d->frame; }

void Layer::setFrame(const Frame *frame) { d->frame = frame; }

const Property *Layer::propertyFromId(Token id) const {
  for (const auto &child : d->properties) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Layer::addProperty(const Property *prop) { d->properties.push_back(prop); }
}
