#include "layer.hpp"
#include "property.hpp"
#include "payload.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

Layer::Layer(Token id) : mId(id) {}

Layer::~Layer() {}

Token Layer::id() const { return mId; }

double Layer::confidence() const { return mConfidence; }

void Layer::setConfidence(double confidence) { mConfidence = confidence; }

const std::vector<Layer *> &Layer::children() const { return mChildren; }

void Layer::addLayer(Layer *child) { mChildren.push_back(child); }

uint32_t Layer::streamId() const { return mStreamId; }

void Layer::setStreamId(uint32_t id) { mStreamId = id; }

const std::vector<Token> &Layer::tags() const { return mTags; }

void Layer::addTag(Token token) { mTags.push_back(token); }

const std::vector<const Payload *> &Layer::payloads() const {
  return mPayloads;
}

void Layer::addPayload(const Payload *payload) { mPayloads.push_back(payload); }

const std::vector<const Property *> &Layer::properties() const {
  return mProperties;
}

const Layer *Layer::parent() const { return mParent; }

void Layer::setParent(const Layer *layer) { mParent = layer; }

const Frame *Layer::frame() const { return mFrame; }

void Layer::setFrame(const Frame *frame) { mFrame = frame; }

const Property *Layer::propertyFromId(Token id) const {
  for (const auto &child : mProperties) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Layer::addProperty(const Property *prop) { mProperties.push_back(prop); }

Token Layer_id(const Layer *layer) { return layer->id(); }

double Layer_confidence(const Layer *layer) { return layer->confidence(); }

void Layer_setConfidence(Layer *layer, double confidence) {
  layer->setConfidence(confidence);
}

const Layer *Layer_parent(const Layer *layer) { return layer->parent(); }

const Frame *Layer_frame(const Layer *layer) { return layer->frame(); }

Layer *Layer_addLayer(Layer *layer, Token id) {
  Layer *child = new Layer(id);
  child->setParent(layer->parent());
  child->setFrame(layer->frame());
  layer->addLayer(child);
  return child;
}

Property *Layer_addProperty(Layer *layer, Token id) {
  Property *prop = new Property(id);
  layer->addProperty(prop);
  return prop;
}

const Property *Layer_propertyFromId(const Layer *layer, Token id) {
  return layer->propertyFromId(id);
}

Payload *Layer_addPayload(Layer *layer, Slice view) {
  Payload *payload = new Payload(view);
  layer->addPayload(payload);
  return payload;
}

const Payload *const *Layer_payloads(const Layer *layer, size_t *size) {
  const auto &payloads = layer->payloads();
  if (size)
    *size = payloads.size();
  return payloads.data();
}

void Layer_addTag(Layer *layer, Token tag) { layer->addTag(tag); }
}
