#include "reader.h"
#include "payload.hpp"
#include <algorithm>
#include <cstring>
#include <vector>

namespace plugkit {

namespace {

Token outOfBoundError() {
  thread_local const Token token = Token_get("!out-of-bounds");
  return token;
}

template <class T>
T readLE(Reader *reader) {
  if (Slice_length(reader->data) - reader->lastRange.end < sizeof(T)) {
    reader->lastError = outOfBoundError();
    return T();
  }
  T value =
      *reinterpret_cast<const T *>(reader->data.begin + reader->lastRange.end);
  reader->lastRange.end += sizeof(T);
  reader->lastRange.begin = reader->lastRange.end - sizeof(T);
  return value;
}

template <class T>
T readBE(Reader *reader) {
  if (Slice_length(reader->data) - reader->lastRange.end < sizeof(T)) {
    reader->lastError = outOfBoundError();
    return T();
  }
  char data[sizeof(T)];
  std::reverse_copy(reader->data.begin + reader->lastRange.end,
                    reader->data.begin + reader->lastRange.end + sizeof(T),
                    data);
  T value = *reinterpret_cast<const T *>(data);
  reader->lastRange.end += sizeof(T);
  reader->lastRange.begin = reader->lastRange.end - sizeof(T);
  return value;
}
} // namespace
void Reader_reset(Reader *reader) { std::memset(reader, 0, sizeof(Reader)); }

Slice Reader_slice(Reader *reader, size_t begin, size_t end) {
  Slice *slice = &reader->data;
  if (slice->begin + reader->lastRange.end + end > slice->end) {
    static const Token outOfBoundError = Token_get("!out-of-bounds");
    reader->lastError = outOfBoundError;
    return Slice();
  }
  Slice subview{slice->begin + reader->lastRange.end, slice->end};
  subview = Slice_slice(subview, begin, end);
  reader->lastRange.end += end;
  reader->lastRange.begin = reader->lastRange.end - (end - begin);
  return subview;
}

Slice Reader_sliceAll(Reader *reader, size_t begin) {
  Slice *slice = &reader->data;
  if (slice->begin + reader->lastRange.end + begin > slice->end) {
    static const Token outOfBoundError = Token_get("!out-of-bounds");
    reader->lastError = outOfBoundError;
    return Slice();
  }
  Slice subview{slice->begin + reader->lastRange.end + begin, slice->end};
  reader->lastRange.begin = reader->lastRange.end + begin;
  reader->lastRange.end = Slice_length(*slice);
  return subview;
}

uint8_t Reader_getUint8(Reader *reader) { return readLE<uint8_t>(reader); }
int8_t Reader_getInt8(Reader *reader) { return readLE<int8_t>(reader); }

uint16_t Reader_getUint16(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<uint16_t>(reader) : readBE<uint16_t>(reader);
}
uint32_t Reader_getUint32(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<uint32_t>(reader) : readBE<uint32_t>(reader);
}
uint64_t Reader_getUint64(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<uint64_t>(reader) : readBE<uint64_t>(reader);
}

int16_t Reader_getInt16(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<int16_t>(reader) : readBE<int16_t>(reader);
}
int32_t Reader_getInt32(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<int32_t>(reader) : readBE<int32_t>(reader);
}
int64_t Reader_getInt64(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<uint64_t>(reader) : readBE<int64_t>(reader);
}

float Reader_getFloat32(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<float>(reader) : readBE<float>(reader);
}
double Reader_getFloat64(Reader *reader, bool littleEndian) {
  return littleEndian ? readLE<double>(reader) : readBE<double>(reader);
}
} // namespace plugkit
