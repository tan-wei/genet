#include "layer.hpp"
#include "attr.hpp"
#include "context.hpp"
#include "payload.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

namespace {
const auto aliasToken = Token_get("--alias");
const auto errorToken = Token_get("--error");
} // namespace

Layer::Layer(Token id) : mId(id) {
  setConfidence(LAYER_CONF_EXACT);
  setParent(nullptr);
}

Layer::~Layer() {}

Token Layer::id() const { return mId; }

LayerConfidence Layer::confidence() const {
  return static_cast<LayerConfidence>((mData >> 4) & 0b11);
}

void Layer::setConfidence(LayerConfidence confidence) {
  mData = ((mData & ~0b110000) | (confidence << 4));
}

Range Layer::range() const { return mRange; }

void Layer::setRange(const Range &range) { mRange = range; }

const std::vector<Layer *> &Layer::layers() const { return mLayers; }

void Layer::addLayer(Layer *child) { mLayers.push_back(child); }

uint8_t Layer::worker() const { return mData & 0b1111; }

void Layer::setWorker(uint8_t id) {
  mData = ((mData & ~0b1111) | (id % LAYER_MAX_WORKER));
}

bool Layer::isRoot() const { return (mData >> 6) & 0b1; }

void Layer::setIsRoot(bool root) {
  mData = ((mData & ~0b1000000) | (root << 6));
}

const std::vector<Token> &Layer::tags() const { return mTags; }

void Layer::addTag(Token token) { mTags.emplace_back(token); }

const std::vector<Payload *> &Layer::payloads() const { return mPayloads; }

void Layer::addPayload(Payload *payload) { mPayloads.push_back(payload); }

const std::vector<Attr *> &Layer::attrs() const { return mAttrs; }

Layer *Layer::parent() const {
  if (isRoot())
    return nullptr;
  return mParent.layer;
}

void Layer::setParent(Layer *layer) {
  setIsRoot(false);
  mParent.layer = layer;
}

const Frame *Layer::frame() const {
  const Layer *layer = this;
  for (; !layer->isRoot() && layer->parent(); layer = layer->parent())
    ;
  if (layer->isRoot()) {
    return layer->mParent.frame;
  }
  return nullptr;
}

void Layer::setFrame(const Frame *frame) {
  setIsRoot(true);
  mParent.frame = frame;
}

const Attr *Layer::attr(Token token) const {
  Token id = token;
  for (const auto &child : mAttrs) {
    if (child->id() == id) {
      if (child->type() == aliasToken) {
        id = child->value().uint64Value();
      } else {
        return child;
      }
    }
  }
  return nullptr;
}

void Layer::addAttr(Attr *attr) { mAttrs.push_back(attr); }

void Layer::removeUnconfidentLayers(Context *ctx, LayerConfidence confidence) {
  for (auto &layer : mLayers) {
    if (layer->confidence() < confidence) {
      for (Attr *attr : layer->mAttrs) {
        Context_deallocAttr(ctx, attr);
      }
      for (Payload *payload : layer->mPayloads) {
        Context_deallocPayload(ctx, payload);
      }
      Context_deallocLayer(ctx, layer);
      layer = nullptr;
    }
  }

  mLayers.erase(std::remove(mLayers.begin(), mLayers.end(), nullptr),
                mLayers.end());
}

Layer *Layer_addLayer(Layer *layer, Context *context, Token id) {
  Layer *child = Context_allocLayer(context, id);
  child->setParent(layer);
  layer->addLayer(child);
  return child;
}

Attr *Layer_addAttr(Layer *layer, Context *context, Token id) {
  Attr *attr = Context_allocAttr(context, id);
  layer->addAttr(attr);
  return attr;
}

void Layer_addAttrAlias(Layer *layer, Context *ctx, Token alias, Token target) {
  Attr *attr = Context_allocAttr(ctx, alias);
  attr->setType(aliasToken);
  attr->setValue(target);
  layer->addAttr(attr);
}

const Attr *Layer_attr(const Layer *layer, Token id) { return layer->attr(id); }

Payload *Layer_addPayload(Layer *layer, Context *context) {
  Payload *payload = Context_allocPayload(context);
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

void Layer_addError(
    Layer *layer, Context *ctx, Token id, const char *msg, size_t length) {
  Attr *attr = Context_allocAttr(ctx, id);
  attr->setType(errorToken);
  if (length > 0) {
    attr->setValue(
        Variant::fromString(ctx->stringPool.get(msg, length), length));
  }
  layer->addAttr(attr);
}

void Layer_addTag(Layer *layer, Context *ctx, Token tag) { layer->addTag(tag); }

} // namespace plugkit
