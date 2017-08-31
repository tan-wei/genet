#include "layer.hpp"
#include "attribute.hpp"
#include "payload.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

Layer::Layer(Token id) : mId(id) {}

Layer::~Layer() {}

Token Layer::id() const { return mId; }

float Layer::confidence() const { return mConfidence; }

void Layer::setConfidence(float confidence) { mConfidence = confidence; }

const std::vector<Layer *> &Layer::layers() const { return mLayers; }

void Layer::addLayer(Layer *child) { mLayers.push_back(child); }

const std::vector<Layer *> &Layer::subLayers() const { return mSubLayers; }

void Layer::addSubLayer(Layer *child) { mSubLayers.push_back(child); }

uint32_t Layer::streamId() const { return mStreamId; }

void Layer::setStreamId(uint32_t id) { mStreamId = id; }

const std::vector<Token> &Layer::tags() const { return mTags; }

void Layer::addTag(Token token) { mTags.push_back(token); }

const std::vector<const Payload *> &Layer::payloads() const {
  return mPayloads;
}

const std::vector<Error> &Layer::errors() const { return mErrors; }

void Layer::addError(Error err) { mErrors.push_back(err); }

void Layer::addPayload(const Payload *payload) { mPayloads.push_back(payload); }

const std::vector<const Attr *> &Layer::properties() const {
  return mProperties;
}

Layer *Layer::parent() const { return mParent; }

void Layer::setParent(Layer *layer) { mParent = layer; }

const Frame *Layer::frame() const { return mFrame; }

void Layer::setFrame(const Frame *frame) { mFrame = frame; }

const Attr *Layer::attr(Token id) const {
  for (const auto &child : mProperties) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Layer::addAttr(const Attr *prop) { mProperties.push_back(prop); }

Token Layer_id(const Layer *layer) { return layer->id(); }

float Layer_confidence(const Layer *layer) { return layer->confidence(); }

void Layer_setConfidence(Layer *layer, float confidence) {
  layer->setConfidence(confidence);
}

const Layer *Layer_parent(const Layer *layer) { return layer->parent(); }

const Frame *Layer_frame(const Layer *layer) { return layer->frame(); }

Layer *Layer_addLayer(Layer *layer, Token id) {
  Layer *child = new Layer(id);
  child->setParent(layer);
  child->setFrame(layer->frame());
  layer->addLayer(child);
  return child;
}

Layer *Layer_addSubLayer(Layer *layer, Token id) {
  Layer *child = new Layer(id);
  child->setParent(layer);
  child->setFrame(layer->frame());
  layer->addSubLayer(child);
  return child;
}

Attr *Layer_addAttr(Layer *layer, Token id) {
  Attr *prop = new Attr(id);
  layer->addAttr(prop);
  return prop;
}

const Attr *Layer_attr(const Layer *layer, Token id) { return layer->attr(id); }

Payload *Layer_addPayload(Layer *layer) {
  Payload *payload = new Payload();
  layer->addPayload(payload);
  return payload;
}

const Payload *const *Layer_payloads(const Layer *layer, size_t *size) {
  const auto &payloads = layer->payloads();
  if (size)
    *size = payloads.size();
  return payloads.data();
}

const Payload *Layer_payload(const Layer *layer) {
  static const Payload empty;
  const auto &payloads = layer->payloads();
  if (payloads.size() > 0) {
    return payloads.front();
  }
  return &empty;
}

void Layer_addTag(Layer *layer, Token tag) { layer->addTag(tag); }

void Layer_addError(Layer *layer, Error err) { layer->addError(err); }
}
