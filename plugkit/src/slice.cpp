#include "slice.h"
#include "error.h"
#include <algorithm>

namespace plugkit {

namespace {

Token outOfBoundError() {
  thread_local const Token token = Token_get("Out of bound");
  return token;
}

template <class T> T readLE(const Slice &slice, size_t offset, Error *err) {
  if (Slice_length(slice) < sizeof(T) + offset) {
    if (err) {
      err->type = outOfBoundError();
    }
    return T();
  }
  return *reinterpret_cast<const T *>(slice.begin + offset);
}

template <class T> T readBE(const Slice &slice, size_t offset, Error *err) {
  if (Slice_length(slice) < sizeof(T) + offset) {
    if (err) {
      err->type = outOfBoundError();
    }
    return T();
  }
  char data[sizeof(T)];
  std::reverse_copy(slice.begin + offset, slice.begin + offset + sizeof(T),
                    data);
  return *reinterpret_cast<const T *>(data);
}
}

size_t Slice_length(Slice slice) { return slice.end - slice.begin; }

Slice Slice_slice(Slice slice, size_t offset, size_t length) {
  Slice subview;
  subview.begin = std::min(slice.begin + offset, slice.end);
  subview.end = std::min(subview.begin + length, slice.end);
  return subview;
}

Slice Slice_sliceAll(Slice slice, size_t offset) {
  return Slice{std::min(slice.begin + offset, slice.end), slice.end};
}

uint8_t Slice_readUint8(Slice slice, size_t offset, Error *err) {
  return readLE<uint8_t>(slice, offset, err);
}

int8_t Slice_readInt8(Slice slice, size_t offset, Error *err) {
  return readLE<int8_t>(slice, offset, err);
}

uint16_t Slice_readUint16BE(Slice slice, size_t offset, Error *err) {
  return readBE<uint16_t>(slice, offset, err);
}

uint32_t Slice_readUint32BE(Slice slice, size_t offset, Error *err) {
  return readBE<uint32_t>(slice, offset, err);
}

uint64_t Slice_readUint64BE(Slice slice, size_t offset, Error *err) {
  return readBE<uint64_t>(slice, offset, err);
}

int16_t Slice_readInt16BE(Slice slice, size_t offset, Error *err) {
  return readBE<int16_t>(slice, offset, err);
}

int32_t Slice_readInt32BE(Slice slice, size_t offset, Error *err) {
  return readBE<int32_t>(slice, offset, err);
}

int64_t Slice_readInt64BE(Slice slice, size_t offset, Error *err) {
  return readBE<int64_t>(slice, offset, err);
}

float Slice_readFloat32BE(Slice slice, size_t offset, Error *err) {
  return readBE<float>(slice, offset, err);
}

double Slice_readFloat64BE(Slice slice, size_t offset, Error *err) {
  return readBE<double>(slice, offset, err);
}

uint16_t Slice_readUint16LE(Slice slice, size_t offset, Error *err) {
  return readLE<uint16_t>(slice, offset, err);
}

uint32_t Slice_readUint32LE(Slice slice, size_t offset, Error *err) {
  return readLE<uint32_t>(slice, offset, err);
}

uint64_t Slice_readUint64LE(Slice slice, size_t offset, Error *err) {
  return readLE<uint64_t>(slice, offset, err);
}

int16_t Slice_readInt16LE(Slice slice, size_t offset, Error *err) {
  return readLE<int16_t>(slice, offset, err);
}

int32_t Slice_readInt32LE(Slice slice, size_t offset, Error *err) {
  return readLE<int32_t>(slice, offset, err);
}

int64_t Slice_readInt64LE(Slice slice, size_t offset, Error *err) {
  return readLE<int64_t>(slice, offset, err);
}

float Slice_readFloat32LE(Slice slice, size_t offset, Error *err) {
  return readLE<float>(slice, offset, err);
}

double Slice_readFloat64LE(Slice slice, size_t offset, Error *err) {
  return readLE<double>(slice, offset, err);
}
}
