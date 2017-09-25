#include "frame.hpp"
#include "wrapper/frame.hpp"

namespace plugkit {

Frame::Frame() {}

Frame::~Frame() {}

Timestamp Frame::timestamp() const { return mTimestamp; }

void Frame::setTimestamp(const Timestamp &timestamp) { mTimestamp = timestamp; }

size_t Frame::length() const { return mLength; }

void Frame::setLength(size_t length) { mLength = length; }

uint32_t Frame::index() const { return mSeq; }

void Frame::setIndex(uint32_t index) { mSeq = index; }

Layer *Frame::rootLayer() const { return mLayer; }

void Frame::setRootLayer(Layer *layer) { mLayer = layer; }

uint32_t Frame::sourceId() const { return mSourceId; }

void Frame::setSourceId(uint32_t id) { mSourceId = id; }

const FrameView *Frame::view() const { return mView; }

void Frame::setView(const FrameView *view) { mView = view; }
} // namespace plugkit
