#include "stream_reader.h"
#include "payload.hpp"
#include <algorithm>
#include <cstring>
#include <vector>

namespace plugkit {

struct StreamReader {
  std::vector<Slice> slices;
  size_t length = 0;
};

StreamReader *StreamReader_create() { return new StreamReader(); }

void StreamReader_destroy(StreamReader *reader) { delete reader; }

size_t StreamReader_length(const StreamReader *reader) {
  return reader->length;
}

void StreamReader_addSlice(StreamReader *reader, Slice slice) {
  reader->slices.push_back(slice);
  reader->length += Slice_length(slice);
}

void StreamReader_addPayload(StreamReader *reader, const Payload *payload) {
  for (const Slice &slice : payload->slices()) {
    StreamReader_addSlice(reader, slice);
  }
}

size_t StreamReader_search(StreamReader *reader,
                           const char *data,
                           size_t length,
                           size_t offset) {
  if (reader->slices.empty() || length == 0) {
    return RANGE_NPOS;
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
          return front + index + length;
        }
      }
    }
    front += sliceLen;
  }
  return RANGE_NPOS;
}

Slice StreamReader_read(StreamReader *reader,
                        char *buffer,
                        size_t length,
                        size_t offset) {
  if (reader->slices.empty()) {
    return Slice{buffer, buffer};
  }
  size_t beginOffset = 0;
  size_t begin = 0;
  for (; begin < reader->slices.size() &&
         (beginOffset += Slice_length(reader->slices[begin])) <= offset;
       ++begin) {
  };
  if (beginOffset <= offset) {
    return Slice{buffer, buffer};
  }
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
      if (!buffer) {
        const char *data = reader->slices[begin].begin + (offset - beginOffset);
        return Slice{data, reader->slices[i].end};
      }
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
    Slice slice = reader->slices[i];
    if (i == begin) {
      slice.begin += (offset - beginOffset);
    }
    size_t capacity = buffer - dst;
    size_t sliceLen = std::min(Slice_length(slice), capacity);
    std::memcpy(dst, slice.begin, sliceLen);
    dst += sliceLen;
  }
  return Slice{buffer, buffer + buflen};
}
} // namespace plugkit
