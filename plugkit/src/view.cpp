#include "view.h"
#include "error.h"
#include <algorithm>

namespace plugkit {

namespace {
const Token outOfBoundError = Token_get("Out of bound");
}

size_t View_length(View view) { return view.end - view.begin; }

View View_slice(View view, size_t offset, size_t length) {
  View subview;
  subview.begin = std::min(view.begin + offset, view.end);
  subview.end = std::min(subview.begin + length, view.end);
  return subview;
}

uint8_t View_readUint8(View view, Error *err) {
  if (View_length(view) < sizeof(uint8_t)) {
    if (err)
      err->type = outOfBoundError;
    return uint8_t();
  }
  return *reinterpret_cast<const uint8_t *>(view.begin);
}

uint16_t View_readUint16LE(View view, Error *err) {
  if (View_length(view) < sizeof(uint16_t)) {
    if (err)
      err->type = outOfBoundError;
    return uint16_t();
  }
  return *reinterpret_cast<const uint16_t *>(view.begin);
}

uint32_t View_readUint32LE(View view, Error *err) {
  if (View_length(view) < sizeof(uint32_t)) {
    if (err)
      err->type = outOfBoundError;
    return uint32_t();
  }
  return *reinterpret_cast<const uint32_t *>(view.begin);
}

int8_t View_readInt8(View view, Error *err) {
  if (View_length(view) < sizeof(int8_t)) {
    if (err)
      err->type = outOfBoundError;
    return int8_t();
  }
  return *reinterpret_cast<const int8_t *>(view.begin);
}

int16_t View_readInt16LE(View view, Error *err) {
  if (View_length(view) < sizeof(int16_t)) {
    if (err)
      err->type = outOfBoundError;
    return int16_t();
  }
  return *reinterpret_cast<const int16_t *>(view.begin);
}

int32_t View_readInt32LE(View view, Error *err) {
  if (View_length(view) < sizeof(int32_t)) {
    if (err)
      err->type = outOfBoundError;
    return int32_t();
  }
  return *reinterpret_cast<const int32_t *>(view.begin);
}
}
