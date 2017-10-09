/// @file
/// Binary sequence
#ifndef PLUGKIT_VIEW_H
#define PLUGKIT_VIEW_H

#include "export.h"
#include "token.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Slice {
  const char *begin;
  const char *end;
} Slice;

inline size_t Slice_length(Slice slice) { return slice.end - slice.begin; }
PLUGKIT_EXPORT Slice Slice_slice(Slice slice, size_t begin, size_t end);
PLUGKIT_EXPORT Slice Slice_sliceAll(Slice slice, size_t begin);

PLUGKIT_EXPORT uint8_t Slice_getUint8(Slice slice, size_t offset, Token *err);
PLUGKIT_EXPORT int8_t Slice_getInt8(Slice slice, size_t offset, Token *err);

PLUGKIT_EXPORT uint16_t Slice_getUint16(Slice slice, size_t offset,
                                        bool littleEndian, Token *err);
PLUGKIT_EXPORT uint32_t Slice_getUint32(Slice slice, size_t offset,
                                        bool littleEndian, Token *err);
PLUGKIT_EXPORT uint64_t Slice_getUint64(Slice slice, size_t offset,
                                        bool littleEndian, Token *err);

PLUGKIT_EXPORT int16_t Slice_getInt16(Slice slice, size_t offset,
                                      bool littleEndian, Token *err);
PLUGKIT_EXPORT int32_t Slice_getInt32(Slice slice, size_t offset,
                                      bool littleEndian, Token *err);
PLUGKIT_EXPORT int64_t Slice_getInt64(Slice slice, size_t offset,
                                      bool littleEndian, Token *err);

PLUGKIT_EXPORT float Slice_getFloat32(Slice slice, size_t offset,
                                      bool littleEndian, Token *err);
PLUGKIT_EXPORT double Slice_getFloat64(Slice slice, size_t offset,
                                       bool littleEndian, Token *err);

PLUGKIT_NAMESPACE_END

#endif
