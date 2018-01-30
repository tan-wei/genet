#include "slice.hpp"
#include <algorithm>

namespace plugkit {

namespace {

Token outOfBoundError() {
  thread_local const Token token = Token_get("!out-of-bounds");
  return token;
}

template <class T>
T readLE(const Slice &slice, size_t offset, Token *err) {
  if (Slice_length(slice) < sizeof(T) + offset) {
    if (err) {
      *err = outOfBoundError();
    }
    return T();
  }
  return *reinterpret_cast<const T *>(slice.begin + offset);
}

template <class T>
T readBE(const Slice &slice, size_t offset, Token *err) {
  if (Slice_length(slice) < sizeof(T) + offset) {
    if (err) {
      *err = outOfBoundError();
    }
    return T();
  }
  char data[sizeof(T)];
  std::reverse_copy(slice.begin + offset, slice.begin + offset + sizeof(T),
                    data);
  return *reinterpret_cast<const T *>(data);
}
} // namespace

Slice Slice_slice(Slice slice, size_t begin, size_t end) {
  Slice subview;
  subview.begin = std::min(slice.begin + begin, slice.end);
  subview.end = std::min(subview.begin + (end - begin), slice.end);
  return subview;
}

Slice Slice_sliceAll(Slice slice, size_t begin) {
  return Slice{std::min(slice.begin + begin, slice.end), slice.end};
}

uint8_t Slice_getUint8(Slice slice, size_t offset, Token *err) {
  return readLE<uint8_t>(slice, offset, err);
}

int8_t Slice_getInt8(Slice slice, size_t offset, Token *err) {
  return readLE<int8_t>(slice, offset, err);
}

uint16_t
Slice_getUint16(Slice slice, size_t offset, bool littleEndian, Token *err) {
  return littleEndian ? readLE<uint16_t>(slice, offset, err)
                      : readBE<uint16_t>(slice, offset, err);
}

uint32_t
Slice_getUint32(Slice slice, size_t offset, bool littleEndian, Token *err) {
  return littleEndian ? readLE<uint32_t>(slice, offset, err)
                      : readBE<uint32_t>(slice, offset, err);
}

uint64_t
Slice_getUint64(Slice slice, size_t offset, bool littleEndian, Token *err) {
  return littleEndian ? readLE<uint64_t>(slice, offset, err)
                      : readBE<uint64_t>(slice, offset, err);
}

int16_t
Slice_getInt16(Slice slice, size_t offset, bool littleEndian, Token *err) {
  return littleEndian ? readLE<int16_t>(slice, offset, err)
                      : readBE<int16_t>(slice, offset, err);
}

int32_t
Slice_getInt32(Slice slice, size_t offset, bool littleEndian, Token *err) {
  return littleEndian ? readLE<int32_t>(slice, offset, err)
                      : readBE<int32_t>(slice, offset, err);
}

int64_t
Slice_getInt64(Slice slice, size_t offset, bool littleEndian, Token *err) {
  return littleEndian ? readLE<int64_t>(slice, offset, err)
                      : readBE<int64_t>(slice, offset, err);
}

float Slice_getFloat32(Slice slice,
                       size_t offset,
                       bool littleEndian,
                       Token *err) {
  return littleEndian ? readLE<float>(slice, offset, err)
                      : readBE<float>(slice, offset, err);
}

double
Slice_getFloat64(Slice slice, size_t offset, bool littleEndian, Token *err) {
  return littleEndian ? readLE<double>(slice, offset, err)
                      : readBE<double>(slice, offset, err);
}
} // namespace plugkit
