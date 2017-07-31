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
  double confidence = 1.0;
  Slice payload;
  std::vector<const Payload *> payloads;
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

double Layer::confidence() const { return d->confidence; }

void Layer::setConfidence(double confidence) { d->confidence = confidence; }

const std::vector<Layer *> &Layer::children() const { return d->children; }

void Layer::addLayer(Layer *child) { d->children.push_back(child); }

uint32_t Layer::streamId() const { return d->streamId; }

void Layer::setStreamId(uint32_t id) { d->streamId = id; }

const Slice &Layer::payload() const { return d->payload; }

const std::vector<Token> &Layer::tags() const { return d->tags; }

void Layer::addTag(Token token) { d->tags.push_back(token); }

void Layer::setPayload(const Slice &payload) { d->payload = payload; }

const std::vector<const Payload *> &Layer::payloads() const {
  return d->payloads;
}

void Layer::addPayload(const Payload *payload) {
  d->payloads.push_back(payload);
}

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

Token Layer_id(const Layer *layer) { return layer->id(); }

double Layer_confidence(const Layer *layer) { return layer->confidence(); }

void Layer_setConfidence(Layer *layer, double confidence) {
  layer->setConfidence(confidence);
}

const Layer *Layer_parent(const Layer *layer) { return layer->parent(); }

const Frame *Layer_frame(const Layer *layer) { return layer->frame(); }

Property *Layer_addProperty(Layer *layer, Token id) {
  Property *prop = new Property(id);
  layer->addProperty(prop);
  return prop;
}

const Property *Layer_propertyFromId(const Layer *layer, Token id) {
  return layer->propertyFromId(id);
}
}
