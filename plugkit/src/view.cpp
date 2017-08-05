#include "view.h"
#include "error.h"
#include <algorithm>

namespace plugkit {

namespace {

Token outOfBoundError() {
  thread_local const Token token = Token_get("Out of bound");
  return token;
}

template <class T> T readLE(const View &view, size_t offset, Error *err) {
  if (View_length(view) < sizeof(T) + offset) {
    if (err) {
      err->type = outOfBoundError();
    }
    return T();
  }
  return *reinterpret_cast<const T *>(view.begin + offset);
}

template <class T> T readBE(const View &view, size_t offset, Error *err) {
  if (View_length(view) < sizeof(T) + offset) {
    if (err) {
      err->type = outOfBoundError();
    }
    return T();
  }
  char data[sizeof(T)];
  std::reverse_copy(view.begin + offset, view.begin + offset + sizeof(T), data);
  return *reinterpret_cast<const T *>(data);
}
}

size_t View_length(View view) { return view.end - view.begin; }

View View_slice(View view, size_t offset, size_t length) {
  View subview;
  subview.begin = std::min(view.begin + offset, view.end);
  subview.end = std::min(subview.begin + length, view.end);
  return subview;
}

View View_sliceAll(View view, size_t offset) {
  return View{std::min(view.begin + offset, view.end), view.end};
}

uint8_t View_readUint8(View view, size_t offset, Error *err) {
  return readLE<uint8_t>(view, offset, err);
}

int8_t View_readInt8(View view, size_t offset, Error *err) {
  return readLE<int8_t>(view, offset, err);
}

uint16_t View_readUint16BE(View view, size_t offset, Error *err) {
  return readBE<uint16_t>(view, offset, err);
}

uint32_t View_readUint32BE(View view, size_t offset, Error *err) {
  return readBE<uint32_t>(view, offset, err);
}

uint64_t View_readUint64BE(View view, size_t offset, Error *err) {
  return readBE<uint64_t>(view, offset, err);
}

int16_t View_readInt16BE(View view, size_t offset, Error *err) {
  return readBE<int16_t>(view, offset, err);
}

int32_t View_readInt32BE(View view, size_t offset, Error *err) {
  return readBE<int32_t>(view, offset, err);
}

int64_t View_readInt64BE(View view, size_t offset, Error *err) {
  return readBE<int64_t>(view, offset, err);
}

float View_readFloat32BE(View view, size_t offset, Error *err) {
  return readBE<float>(view, offset, err);
}

double View_readFloat64BE(View view, size_t offset, Error *err) {
  return readBE<double>(view, offset, err);
}

uint16_t View_readUint16LE(View view, size_t offset, Error *err) {
  return readLE<uint16_t>(view, offset, err);
}

uint32_t View_readUint32LE(View view, size_t offset, Error *err) {
  return readLE<uint32_t>(view, offset, err);
}

uint64_t View_readUint64LE(View view, size_t offset, Error *err) {
  return readLE<uint64_t>(view, offset, err);
}

int16_t View_readInt16LE(View view, size_t offset, Error *err) {
  return readLE<int16_t>(view, offset, err);
}

int32_t View_readInt32LE(View view, size_t offset, Error *err) {
  return readLE<int32_t>(view, offset, err);
}

int64_t View_readInt64LE(View view, size_t offset, Error *err) {
  return readLE<int64_t>(view, offset, err);
}

float View_readFloat32LE(View view, size_t offset, Error *err) {
  return readLE<float>(view, offset, err);
}

double View_readFloat64LE(View view, size_t offset, Error *err) {
  return readLE<double>(view, offset, err);
}
}
