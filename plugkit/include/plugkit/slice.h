/// @file
/// Binary sequence
#ifndef PLUGKIT_VIEW_H
#define PLUGKIT_VIEW_H

#include <cstddef>
#include <cstdint>

extern "C" {

namespace plugkit {

struct Error;

struct Slice {
  const char *begin;
  const char *end;
};

size_t Slice_length(Slice slice);
Slice Slice_slice(Slice slice, size_t offset, size_t length);
Slice Slice_sliceAll(Slice slice, size_t offset);

uint8_t Slice_readUint8(Slice slice, size_t offset, Error *err);
int8_t Slice_readInt8(Slice slice, size_t offset, Error *err);

uint16_t Slice_readUint16BE(Slice slice, size_t offset, Error *err);
uint32_t Slice_readUint32BE(Slice slice, size_t offset, Error *err);
uint64_t Slice_readUint64BE(Slice slice, size_t offset, Error *err);

int16_t Slice_readInt16BE(Slice slice, size_t offset, Error *err);
int32_t Slice_readInt32BE(Slice slice, size_t offset, Error *err);
int64_t Slice_readInt64BE(Slice slice, size_t offset, Error *err);

float Slice_readFloat32BE(Slice slice, size_t offset, Error *err);
double Slice_readFloat64BE(Slice slice, size_t offset, Error *err);

uint16_t Slice_readUint16LE(Slice slice, size_t offset, Error *err);
uint32_t Slice_readUint32LE(Slice slice, size_t offset, Error *err);
uint64_t Slice_readUint64LE(Slice slice, size_t offset, Error *err);

int16_t Slice_readInt16LE(Slice slice, size_t offset, Error *err);
int32_t Slice_readInt32LE(Slice slice, size_t offset, Error *err);
int64_t Slice_readInt64LE(Slice slice, size_t offset, Error *err);

float Slice_readFloat32LE(Slice slice, size_t offset, Error *err);
double Slice_readFloat64LE(Slice slice, size_t offset, Error *err);
}
}

#endif
