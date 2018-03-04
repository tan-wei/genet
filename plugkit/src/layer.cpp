#include "layer.hpp"
#include "attr.hpp"
#include "context.hpp"
#include "error.hpp"
#include "payload.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

namespace {
const auto aliasToken = Token_get("--alias");
const auto dynamicToken = Token_get("--dynamic");
} // namespace

Layer::Layer(Token id) : mId(id) { setConfidence(LAYER_CONF_EXACT); }

Layer::~Layer() {}

Token Layer::id() const { return mId; }

LayerConfidence Layer::confidence() const {
  return static_cast<LayerConfidence>((mData >> 4) & 0b00000011);
}

void Layer::setConfidence(LayerConfidence confidence) {
  mData = ((mData & 0b11001111) | (confidence << 4));
}

Range Layer::range() const { return mRange; }

void Layer::setRange(const Range &range) { mRange = range; }

const std::vector<Layer *> &Layer::layers() const { return mLayers; }

void Layer::addLayer(Layer *child) { mLayers.push_back(child); }

const std::vector<Layer *> &Layer::subLayers() const { return mSubLayers; }

void Layer::addSubLayer(Layer *child) { mSubLayers.push_back(child); }

uint8_t Layer::worker() const { return mData & 0b00001111; }

void Layer::setWorker(uint8_t id) {
  mData = ((mData & 0b11110000) | (id % LAYER_MAX_WORKER));
}

const std::vector<Token> &Layer::tags() const { return mTags; }

void Layer::addTag(Token token) { mTags.emplace_back(token); }

const std::vector<Payload *> &Layer::payloads() const { return mPayloads; }

void Layer::addPayload(Payload *payload) { mPayloads.push_back(payload); }

const std::vector<Error *> &Layer::errors() const { return mErrors; }

void Layer::addError(Error *error) { mErrors.push_back(error); }

const std::vector<Attr *> &Layer::attrs() const { return mAttrs; }

Layer *Layer::parent() const { return mParent; }

void Layer::setParent(Layer *layer) { mParent = layer; }

const Layer *Layer::prev() const { return mPrev; }

void Layer::setPrev(const Layer *layer) { mPrev = layer; }

const Layer *Layer::next() const { return mNext; }

void Layer::setNext(const Layer *layer) { mNext = layer; }

const Frame *Layer::frame() const { return mFrame; }

void Layer::setFrame(const Frame *frame) { mFrame = frame; }

const Attr *Layer::attr(Token token, const char *name) const {
  Token id = token;
  bool matchDynamicName = false;
  for (const auto &child : mAttrs) {
    if (matchDynamicName) {
      matchDynamicName = false;
      if (child->id() == dynamicToken) {
        return child;
      }
    } else if (child->id() == id) {
      if (child->type() == aliasToken) {
        id = child->value().uint64Value();
      } else if (child->type() == dynamicToken) {
        if (name && child->value().string() == name) {
          matchDynamicName = true;
          continue;
        }
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
      for (Error *error : layer->mErrors) {
        Context_deallocError(ctx, error);
      }
      Context_deallocLayer(ctx, layer);
      layer = nullptr;
    }
  }

  for (auto &layer : mSubLayers) {
    if (layer->confidence() < confidence) {
      for (Attr *attr : layer->mAttrs) {
        Context_deallocAttr(ctx, attr);
      }
      for (Payload *payload : layer->mPayloads) {
        Context_deallocPayload(ctx, payload);
      }
      for (Error *error : layer->mErrors) {
        Context_deallocError(ctx, error);
      }
      Context_deallocLayer(ctx, layer);
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

Range Layer_range(const Layer *layer) { return layer->range(); }

void Layer_setRange(Layer *layer, Range range) { layer->setRange(range); }

const Layer *Layer_parent(const Layer *layer) { return layer->parent(); }

const Frame *Layer_frame(const Layer *layer) { return layer->frame(); }

Layer *Layer_addLayer(Layer *layer, Context *context, Token id) {
  Layer *child = Context_allocLayer(context, id);
  child->setParent(layer);
  child->setFrame(layer->frame());
  layer->addLayer(child);
  return child;
}

Layer *Layer_addSubLayer(Layer *layer, Context *context, Token id) {
  Layer *child = Context_allocLayer(context, id);
  child->setParent(layer);
  child->setFrame(layer->frame());
  layer->addSubLayer(child);
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

Attr *Layer_addAttrStr(
    Layer *layer, Context *ctx, Token prefix, const char *name, size_t length) {
  Attr *dynAttr = Context_allocAttr(ctx, prefix);
  dynAttr->setType(dynamicToken);
  dynAttr->setValue(Variant(std::string(name, length).c_str()));
  layer->addAttr(dynAttr);
  Attr *attr = Context_allocAttr(ctx, dynamicToken);
  layer->addAttr(attr);
  return attr;
}

Attr *Layer_addAttrStrRef(
    Layer *layer, Context *ctx, Token prefix, const char *name, size_t length) {
  Attr *dynAttr = Context_allocAttr(ctx, prefix);
  dynAttr->setType(dynamicToken);
  dynAttr->setValue(Variant(name, length));
  layer->addAttr(dynAttr);
  Attr *attr = Context_allocAttr(ctx, dynamicToken);
  layer->addAttr(attr);
  return attr;
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

Error *Layer_addError(Layer *layer, Context *ctx, Token id) {
  Error *error = Context_allocError(ctx, id);
  layer->addError(error);
  return error;
}

const Error *const *Layer_errors(const Layer *layer, size_t *size) {
  const auto &errors = layer->errors();
  if (size)
    *size = errors.size();
  if (errors.empty()) {
    static const Error empty{Token_null(), Token_null()};
    static const Error *ptr = &empty;
    return &ptr;
  }
  return errors.data();
}

void Layer_addTag(Layer *layer, Context *ctx, Token tag) { layer->addTag(tag); }

} // namespace plugkit
