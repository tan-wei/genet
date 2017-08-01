#include "frame.hpp"
#include "wrapper/frame.hpp"

namespace plugkit {

Frame::Frame() {}

Frame::~Frame() {}

Timestamp Frame::timestamp() const { return data.timestamp; }

void Frame::setTimestamp(const Timestamp &timestamp) {
  data.timestamp = timestamp;
}

size_t Frame::length() const { return data.length; }

void Frame::setLength(size_t length) { data.length = length; }

uint32_t Frame::index() const { return data.seq; }

void Frame::setIndex(uint32_t index) { data.seq = index; }

Layer *Frame::rootLayer() const { return data.layer; }

void Frame::setRootLayer(Layer *layer) { data.layer = layer; }

uint32_t Frame::sourceId() const { return data.sourceId; }

void Frame::setSourceId(uint32_t id) { data.sourceId = id; }
}
