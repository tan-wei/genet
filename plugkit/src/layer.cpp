#include "layer.hpp"
#include "property.hpp"
#include "slice.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <regex>

namespace plugkit {

class Layer::Private {
public:
  Private(const minins &ns);

public:
  minins ns;
  minins streamId;
  std::string summary;
  std::string error;
  std::pair<uint32_t, uint32_t> range;
  double confidence = 1.0;
  Slice payload;
  const Layer *parent = nullptr;
  const Frame *frame = nullptr;
  std::vector<Layer *> children;
  std::vector<const Property *> properties;
};

Layer::Private::Private(const minins &ns) : ns(ns) {}

Layer::Layer() : d(new Private(minins())) {}

Layer::Layer(const minins &ns) : d(new Private(ns)) {}

Layer::~Layer() {}

miniid Layer::id() const { return d->ns.primary(); }

minins Layer::ns() const { return d->ns; }

void Layer::setNs(const minins &ns) { d->ns = ns; }

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

const std::vector<Layer *> &Layer::children() const { return d->children; }

void Layer::addChild(Layer *child) { d->children.push_back(child); }

minins Layer::streamId() const { return d->streamId; }

void Layer::setStreamId(const minins &id) { d->streamId = id; }

const Slice &Layer::payload() const { return d->payload; }

void Layer::setPayload(const Slice &payload) { d->payload = payload; }

const std::vector<const Property *> &Layer::properties() const {
  return d->properties;
}

const Layer *Layer::parent() const { return d->parent; }

void Layer::setParent(const Layer *layer) { d->parent = layer; }

const Frame *Layer::frame() const { return d->frame; }

void Layer::setFrame(const Frame *frame) { d->frame = frame; }

const Property *Layer::propertyFromId(miniid id) const {
  for (const auto &child : d->properties) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Layer::addProperty(const Property *prop) { d->properties.push_back(prop); }

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
