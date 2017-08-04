/// @file
/// Binary sequence
#ifndef PLUGKIT_VIEW_H
#define PLUGKIT_VIEW_H

#include <cstddef>
#include <cstdint>

extern "C" {

namespace plugkit {

struct Error;

struct View {
  const char *begin;
  const char *end;
};

size_t View_length(View view);
View View_slice(View view, size_t offset, size_t length);

uint8_t View_readUint8(View view, Error *err);
uint16_t View_readUint16LE(View view, Error *err);
uint32_t View_readUint32LE(View view, Error *err);

int8_t View_readInt8(View view, Error *err);
int16_t View_readInt16LE(View view, Error *err);
int32_t View_readInt32LE(View view, Error *err);
}
}

#endif
