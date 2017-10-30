#include "layer.hpp"
#include "attr.hpp"
#include "payload.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

Layer::Layer(Token id) : mId(id) { setConfidence(LAYER_CONF_EXACT); }

Layer::~Layer() {
  for (const auto &payload : mPayloads) {
    delete payload;
  }
  for (const auto &layer : mLayers) {
    delete layer;
  }
  for (const auto &layer : mSubLayers) {
    delete layer;
  }
  for (const auto &attr : mAttrs) {
    delete attr;
  }
}

Token Layer::id() const { return mId; }

LayerConfidence Layer::confidence() const {
  return static_cast<LayerConfidence>((mData >> 4) & 0x3);
}

void Layer::setConfidence(LayerConfidence confidence) {
  mData = ((mData & 0xcf) | (confidence << 4));
}

const std::vector<Layer *> &Layer::layers() const { return mLayers; }

void Layer::addLayer(Layer *child) { mLayers.push_back(child); }

const std::vector<Layer *> &Layer::subLayers() const { return mSubLayers; }

void Layer::addSubLayer(Layer *child) { mSubLayers.push_back(child); }

uint8_t Layer::worker() const { return mData & 0xf; }

void Layer::setWorker(uint8_t id) {
  mData = ((mData & 0xf0) | (id % LAYER_MAX_WORKER));
}

const std::vector<Token> &Layer::tags() const { return mTags; }

void Layer::addTag(Token token) { mTags.push_back(token); }

const std::vector<const Payload *> &Layer::payloads() const {
  return mPayloads;
}

void Layer::addPayload(const Payload *payload) { mPayloads.push_back(payload); }

const std::vector<const Attr *> &Layer::attrs() const { return mAttrs; }

Layer *Layer::parent() const { return mParent; }

void Layer::setParent(Layer *layer) { mParent = layer; }

const Frame *Layer::frame() const { return mFrame; }

void Layer::setFrame(const Frame *frame) { mFrame = frame; }

const Attr *Layer::attr(Token id) const {
  for (const auto &child : mAttrs) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Layer::addAttr(const Attr *prop) { mAttrs.push_back(prop); }

void Layer::removeUnconfidentLayers(LayerConfidence confidence) {
  for (auto &layer : mLayers) {
    if (layer->confidence() < confidence) {
      delete layer;
      layer = nullptr;
    }
  }

  for (auto &layer : mSubLayers) {
    if (layer->confidence() < confidence) {
      delete layer;
      layer = nullptr;
    }
  }

  mLayers.erase(std::remove(mLayers.begin(), mLayers.end(), nullptr),
                mLayers.end());

  mSubLayers.erase(std::remove(mSubLayers.begin(), mSubLayers.end(), nullptr),
                   mSubLayers.end());
}

Token Layer_id(const Layer *layer) { return layer->id(); }

uint8_t Layer_worker(const Layer *layer) { return layer->worker(); }

void Layer_setWorker(Layer *layer, uint8_t id) { layer->setWorker(id); }

LayerConfidence Layer_confidence(const Layer *layer) {
  return layer->confidence();
}

void Layer_setConfidence(Layer *layer, LayerConfidence confidence) {
  layer->setConfidence(confidence);
}

const Layer *Layer_parent(const Layer *layer) { return layer->parent(); }

const Frame *Layer_frame(const Layer *layer) { return layer->frame(); }

Layer *Layer_addLayer(Context *context, Layer *layer, Token id) {
  Layer *child = new Layer(id);
  child->setParent(layer);
  child->setFrame(layer->frame());
  layer->addLayer(child);
  return child;
}

Layer *Layer_addSubLayer(Context *context, Layer *layer, Token id) {
  Layer *child = new Layer(id);
  child->setParent(layer);
  child->setFrame(layer->frame());
  layer->addSubLayer(child);
  return child;
}

Attr *Layer_addAttr(Context *context, Layer *layer, Token id) {
  Attr *prop = new Attr(id);
  layer->addAttr(prop);
  return prop;
}

const Attr *Layer_attr(const Layer *layer, Token id) { return layer->attr(id); }

Payload *Layer_addPayload(Context *context, Layer *layer) {
  Payload *payload = new Payload();
  layer->addPayload(payload);
  return payload;
}

const Payload *const *Layer_payloads(const Layer *layer, size_t *size) {
  const auto &payloads = layer->payloads();
  if (size)
    *size = payloads.size();
  if (payloads.empty()) {
    static const Payload empty;
    static const Payload *ptr = &empty;
    return &ptr;
  }
  return payloads.data();
}

void Layer_addTag(Layer *layer, Token tag) { layer->addTag(tag); }

} // namespace plugkit
