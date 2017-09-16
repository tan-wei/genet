/// @file
/// Binary sequence
#ifndef PLUGKIT_VIEW_H
#define PLUGKIT_VIEW_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Error Error;

typedef struct Slice {
  const char *begin;
  const char *end;
} Slice;

inline size_t Slice_length(Slice slice) { return slice.end - slice.begin; }
PLUGKIT_EXPORT Slice Slice_slice(Slice slice, size_t offset, size_t length);
PLUGKIT_EXPORT Slice Slice_sliceAll(Slice slice, size_t offset);

PLUGKIT_EXPORT uint8_t Slice_getUint8(Slice slice, size_t offset, Error *err);
PLUGKIT_EXPORT int8_t Slice_getInt8(Slice slice, size_t offset, Error *err);

PLUGKIT_EXPORT uint16_t Slice_getUint16(Slice slice, size_t offset,
                                        bool littleEndian, Error *err);
PLUGKIT_EXPORT uint32_t Slice_getUint32(Slice slice, size_t offset,
                                        bool littleEndian, Error *err);
PLUGKIT_EXPORT uint64_t Slice_getUint64(Slice slice, size_t offset,
                                        bool littleEndian, Error *err);

PLUGKIT_EXPORT int16_t Slice_getInt16(Slice slice, size_t offset,
                                      bool littleEndian, Error *err);
PLUGKIT_EXPORT int32_t Slice_getInt32(Slice slice, size_t offset,
                                      bool littleEndian, Error *err);
PLUGKIT_EXPORT int64_t Slice_getInt64(Slice slice, size_t offset,
                                      bool littleEndian, Error *err);

PLUGKIT_EXPORT float Slice_getFloat32(Slice slice, size_t offset,
                                      bool littleEndian, Error *err);
PLUGKIT_EXPORT double Slice_getFloat64(Slice slice, size_t offset,
                                       bool littleEndian, Error *err);

PLUGKIT_NAMESPACE_END

#endif
