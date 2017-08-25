#include "reader.h"
#include "payload.hpp"
#include <algorithm>
#include <vector>
#include <cstring>

namespace plugkit {

namespace {

Token outOfBoundError() {
  thread_local const Token token = Token_get("Out of bound");
  return token;
}

template <class T> T readLE(Reader *reader) {
  if (Slice_length(reader->slice) - reader->lastRange.end < sizeof(T)) {
    reader->lastError.type = outOfBoundError();
    return T();
  }
  T value =
      *reinterpret_cast<const T *>(reader->slice.begin + reader->lastRange.end);
  reader->lastRange.end += sizeof(T);
  reader->lastRange.begin = reader->lastRange.end - sizeof(T);
  return value;
}

template <class T> T readBE(Reader *reader) {
  if (Slice_length(reader->slice) - reader->lastRange.end < sizeof(T)) {
    reader->lastError.type = outOfBoundError();
    return T();
  }
  char data[sizeof(T)];
  std::reverse_copy(reader->slice.begin + reader->lastRange.end,
                    reader->slice.begin + reader->lastRange.end + sizeof(T),
                    data);
  T value = *reinterpret_cast<const T *>(data);
  reader->lastRange.end += sizeof(T);
  reader->lastRange.begin = reader->lastRange.end - sizeof(T);
  return value;
}
}
void Reader_reset(Reader *reader) { std::memset(reader, 0, sizeof(Reader)); }

Slice Reader_slice(Reader *reader, size_t offset, size_t length) {
  Slice *slice = &reader->slice;
  if (slice->begin + reader->lastRange.end + offset + length > slice->end) {
    static const Token outOfBoundError = Token_get("Out of bound");
    reader->lastError.type = outOfBoundError;
    return Slice();
  }
  Slice subview{slice->begin + reader->lastRange.end, slice->end};
  subview = Slice_slice(subview, offset, length);
  reader->lastRange.end += offset + length;
  reader->lastRange.begin = reader->lastRange.end - length;
  return subview;
}

Slice Reader_sliceAll(Reader *reader, size_t offset) {
  Slice *slice = &reader->slice;
  if (slice->begin + reader->lastRange.end + offset > slice->end) {
    static const Token outOfBoundError = Token_get("Out of bound");
    reader->lastError.type = outOfBoundError;
    return Slice();
  }
  Slice subview{slice->begin + reader->lastRange.end + offset, slice->end};
  reader->lastRange.begin = reader->lastRange.end + offset;
  reader->lastRange.end = Slice_length(*slice);
  return subview;
}

uint8_t Reader_readUint8(Reader *reader) { return readLE<uint8_t>(reader); }
int8_t Reader_readInt8(Reader *reader) { return readLE<int8_t>(reader); }

uint16_t Reader_readUint16BE(Reader *reader) {
  return readBE<uint16_t>(reader);
}
uint32_t Reader_readUint32BE(Reader *reader) {
  return readBE<uint32_t>(reader);
}
uint64_t Reader_readUint64BE(Reader *reader) {
  return readBE<uint64_t>(reader);
}

int16_t Reader_readInt16BE(Reader *reader) { return readBE<int16_t>(reader); }
int32_t Reader_readInt32BE(Reader *reader) { return readBE<int32_t>(reader); }
int64_t Reader_readInt64BE(Reader *reader) { return readBE<int64_t>(reader); }

float Reader_readFloat32BE(Reader *reader) { return readBE<float>(reader); }
double Reader_readFloat64BE(Reader *reader) { return readBE<double>(reader); }

uint16_t Reader_readUint16LE(Reader *reader) {
  return readLE<uint16_t>(reader);
}
uint32_t Reader_readUint32LE(Reader *reader) {
  return readLE<uint32_t>(reader);
}
uint64_t Reader_readUint64LE(Reader *reader) {
  return readLE<uint64_t>(reader);
}

int16_t Reader_readInt16LE(Reader *reader) { return readLE<int16_t>(reader); }
int32_t Reader_readInt32LE(Reader *reader) { return readLE<int32_t>(reader); }
int64_t Reader_readInt64LE(Reader *reader) { return readLE<int64_t>(reader); }

float Reader_readFloat32LE(Reader *reader) { return readLE<float>(reader); }
double Reader_readFloat64LE(Reader *reader) { return readLE<double>(reader); }

struct StreamReader {
  std::vector<Slice> slices;
  size_t length = 0;
};

StreamReader *StreamReader_create() { return new StreamReader(); }

void StreamReader_destroy(StreamReader *reader) { delete reader; }

void StreamReader_addSlice(StreamReader *reader, Slice slice) {
  reader->slices.push_back(slice);
  reader->length += Slice_length(slice);
}

void StreamReader_addPayload(StreamReader *reader, const Payload *payload) {
  for (const Slice &slice : payload->slices()) {
    StreamReader_addSlice(reader, slice);
  }
}

Range StreamReader_search(StreamReader *reader, const char *data, size_t length,
                          size_t offset) {
  if (reader->slices.empty() || length == 0) {
    return Range{0, 0};
  }
  size_t beginOffset = 0;
  size_t begin = 0;
  for (; begin < reader->slices.size() &&
         (beginOffset += Slice_length(reader->slices[begin])) <= offset;
       ++begin)
    ;
  size_t beginLen = Slice_length(reader->slices[begin]);
  beginOffset -= beginLen;
  size_t front = beginOffset;

  std::unique_ptr<char[]> buf(new char[length]);

  for (size_t i = begin; i < reader->slices.size(); ++i) {
    const Slice &slice = reader->slices[i];
    size_t sliceLen = Slice_length(slice);
    size_t index = 0;
    if (i == begin) {
      index = offset - beginOffset;
    }
    for (; index < sliceLen; ++index) {
      if (slice.begin[index] == data[0]) {
        const Slice &window =
            StreamReader_read(reader, &buf[0], length, front + index);
        if (Slice_length(window) == length &&
            std::memcmp(window.begin, data, length) == 0) {
          front += index;
          return Range{front, front + length};
        }
      }
    }
    front += sliceLen;
  }

  return Range{0, 0};
}

Slice StreamReader_read(StreamReader *reader, char *buffer, size_t length,
                        size_t offset) {
  if (reader->slices.empty()) {
    return Slice{buffer, buffer};
  }
  size_t beginOffset = 0;
  size_t begin = 0;
  for (; begin < reader->slices.size() &&
         (beginOffset += Slice_length(reader->slices[begin])) <= offset;
       ++begin)
    ;
  beginOffset -= Slice_length(reader->slices[begin]);
  size_t endOffset = beginOffset;
  size_t end = begin;
  for (; end < reader->slices.size() &&
         (endOffset += Slice_length(reader->slices[end])) < offset + length;
       ++end)
    ;
  bool continuous = true;
  for (size_t i = begin; i < end; ++i) {
    if (reader->slices[i].end != reader->slices[i + 1].begin) {
      continuous = false;
      break;
    }
  }
  size_t buflen = std::min(length, endOffset - beginOffset);
  if (continuous) {
    const char *data = reader->slices[begin].begin + (offset - beginOffset);
    return Slice{data, data + buflen};
  }
  char *dst = buffer;
  for (size_t i = begin; i <= end; ++i) {
    const Slice &slice = reader->slices[i];
    size_t sliceLen = Slice_length(slice);
    std::memcpy(dst, slice.begin, sliceLen);
    dst += sliceLen;
  }
  return Slice{buffer, buffer + buflen};
}
}
