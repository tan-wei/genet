#include "reader.h"
#include <algorithm>
#include <cstring>

namespace plugkit {

namespace {

Token outOfBoundError() {
  thread_local const Token token = Token_get("Out of bound");
  return token;
}

template <class T> T readLE(Reader *reader) {
  if (View_length(reader->view) - reader->lastRange.end < sizeof(T)) {
    reader->lastError.type = outOfBoundError();
    return T();
  }
  T value =
      *reinterpret_cast<const T *>(reader->view.begin + reader->lastRange.end);
  reader->lastRange.end += sizeof(T);
  reader->lastRange.begin = reader->lastRange.end - sizeof(T);
  return value;
}

template <class T> T readBE(Reader *reader) {
  if (View_length(reader->view) - reader->lastRange.end < sizeof(T)) {
    reader->lastError.type = outOfBoundError();
    return T();
  }
  char data[sizeof(T)];
  std::reverse_copy(reader->view.begin + reader->lastRange.end,
                    reader->view.begin + reader->lastRange.end + sizeof(T),
                    data);
  T value = *reinterpret_cast<const T *>(data);
  reader->lastRange.end += sizeof(T);
  reader->lastRange.begin = reader->lastRange.end - sizeof(T);
  return value;
}
}
void Reader_reset(Reader *reader) { std::memset(reader, 0, sizeof(Reader)); }

View Reader_slice(Reader *reader, size_t offset, size_t length) {
  View *view = &reader->view;
  if (view->begin + reader->lastRange.end + offset + length > view->end) {
    static const Token outOfBoundError = Token_get("Out of bound");
    reader->lastError.type = outOfBoundError;
    return View();
  }
  View subview{view->begin + reader->lastRange.end, view->end};
  subview = View_slice(subview, offset, length);
  reader->lastRange.end += offset + length;
  reader->lastRange.begin = reader->lastRange.end - length;
  return subview;
}

View Reader_sliceAll(Reader *reader, size_t offset) {
  View *view = &reader->view;
  if (view->begin + reader->lastRange.end + offset > view->end) {
    static const Token outOfBoundError = Token_get("Out of bound");
    reader->lastError.type = outOfBoundError;
    return View();
  }
  View subview{view->begin + reader->lastRange.end + offset, view->end};
  reader->lastRange.begin = reader->lastRange.end + offset;
  reader->lastRange.end = View_length(*view);
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
}
