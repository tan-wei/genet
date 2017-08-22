/// @file
/// Binary sequence
#ifndef PLUGKIT_VIEW_H
#define PLUGKIT_VIEW_H

#include <stddef.h>
#include <stdint.h>
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Error Error;

typedef struct Slice {
  const char *begin;
  const char *end;
} Slice;

PLUGKIT_EXPORT size_t Slice_length(Slice slice);
PLUGKIT_EXPORT Slice Slice_slice(Slice slice, size_t offset, size_t length);
PLUGKIT_EXPORT Slice Slice_sliceAll(Slice slice, size_t offset);

PLUGKIT_EXPORT uint8_t Slice_readUint8(Slice slice, size_t offset, Error *err);
PLUGKIT_EXPORT int8_t Slice_readInt8(Slice slice, size_t offset, Error *err);

PLUGKIT_EXPORT uint16_t Slice_readUint16BE(Slice slice, size_t offset,
                                           Error *err);
PLUGKIT_EXPORT uint32_t Slice_readUint32BE(Slice slice, size_t offset,
                                           Error *err);
PLUGKIT_EXPORT uint64_t Slice_readUint64BE(Slice slice, size_t offset,
                                           Error *err);

PLUGKIT_EXPORT int16_t Slice_readInt16BE(Slice slice, size_t offset,
                                         Error *err);
PLUGKIT_EXPORT int32_t Slice_readInt32BE(Slice slice, size_t offset,
                                         Error *err);
PLUGKIT_EXPORT int64_t Slice_readInt64BE(Slice slice, size_t offset,
                                         Error *err);

PLUGKIT_EXPORT float Slice_readFloat32BE(Slice slice, size_t offset,
                                         Error *err);
PLUGKIT_EXPORT double Slice_readFloat64BE(Slice slice, size_t offset,
                                          Error *err);

PLUGKIT_EXPORT uint16_t Slice_readUint16LE(Slice slice, size_t offset,
                                           Error *err);
PLUGKIT_EXPORT uint32_t Slice_readUint32LE(Slice slice, size_t offset,
                                           Error *err);
PLUGKIT_EXPORT uint64_t Slice_readUint64LE(Slice slice, size_t offset,
                                           Error *err);

PLUGKIT_EXPORT int16_t Slice_readInt16LE(Slice slice, size_t offset,
                                         Error *err);
PLUGKIT_EXPORT int32_t Slice_readInt32LE(Slice slice, size_t offset,
                                         Error *err);
PLUGKIT_EXPORT int64_t Slice_readInt64LE(Slice slice, size_t offset,
                                         Error *err);

PLUGKIT_EXPORT float Slice_readFloat32LE(Slice slice, size_t offset,
                                         Error *err);
PLUGKIT_EXPORT double Slice_readFloat64LE(Slice slice, size_t offset,
                                          Error *err);

PLUGKIT_NAMESPACE_END

#endif
