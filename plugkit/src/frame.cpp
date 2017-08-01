#include "frame.hpp"
#include "wrapper/frame.hpp"

namespace plugkit {

Frame::Frame() {}

Frame::~Frame() {}

void Frame::setTimestamp(const Timestamp &timestamp) { timestamp_ = timestamp; }

size_t Frame::length() const { return length_; }

void Frame::setLength(size_t length) { length_ = length; }

uint32_t Frame::index() const { return seq_; }

void Frame::setIndex(uint32_t index) { seq_ = index; }

Layer *Frame::rootLayer() const { return layer_; }

void Frame::setRootLayer(Layer *layer) { layer_ = layer; }

uint32_t Frame::sourceId() const { return sourceId_; }

void Frame::setSourceId(uint32_t id) { sourceId_ = id; }
}
