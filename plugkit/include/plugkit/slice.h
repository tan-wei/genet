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

/// Return the length of the slice.
inline size_t Slice_length(Slice slice) { return slice.end - slice.begin; }

/// Return a `Slice` cropped by `begin` and `end` indices.
PLUGKIT_EXPORT Slice Slice_slice(Slice slice, size_t begin, size_t end);

/// Return a `Slice` offset by `begin` bytes.
///
/// Equivalent to `Slice_slice(slice, begin, Slice_length(slice))`.
PLUGKIT_EXPORT Slice Slice_sliceAll(Slice slice, size_t begin);

/// Read a byte from the `offset` index of the slice
/// and return it as `uint8_t`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint8_t Slice_getUint8(Slice slice, size_t offset, Token *err);

/// Read a byte from the `offset` index of the slice
/// and return it as `int8_t`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT int8_t Slice_getInt8(Slice slice, size_t offset, Token *err);

/// Read 2 bytes from the `offset` index of the slice
/// and return it as `uint16_t`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint16_t Slice_getUint16(Slice slice,
                                        size_t offset,
                                        bool littleEndian,
                                        Token *err);

/// Read 4 bytes from the `offset` index of the slice
/// and return it as `uint32_t`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint32_t Slice_getUint32(Slice slice,
                                        size_t offset,
                                        bool littleEndian,
                                        Token *err);

/// Read 8 bytes from the `offset` index of the slice
/// and return it as `uint64_t`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint64_t Slice_getUint64(Slice slice,
                                        size_t offset,
                                        bool littleEndian,
                                        Token *err);

/// Read 2 bytes from the `offset` index of the slice
/// and return it as `int16_t`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT int16_t Slice_getInt16(Slice slice,
                                      size_t offset,
                                      bool littleEndian,
                                      Token *err);

/// Read 4 bytes from the `offset` index of the slice
/// and return it as `int32_t`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT int32_t Slice_getInt32(Slice slice,
                                      size_t offset,
                                      bool littleEndian,
                                      Token *err);

/// Read 8 bytes from the `offset` index of the slice
/// and return it as `int64_t`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT int64_t Slice_getInt64(Slice slice,
                                      size_t offset,
                                      bool littleEndian,
                                      Token *err);

/// Read 4 bytes from the `offset` index of the slice
/// and return it as `float`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT float
Slice_getFloat32(Slice slice, size_t offset, bool littleEndian, Token *err);

/// Read 8 bytes from the `offset` index of the slice
/// and return it as `double`.
///
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `err` is set to `!out-of-bounds`
PLUGKIT_EXPORT double
Slice_getFloat64(Slice slice, size_t offset, bool littleEndian, Token *err);

PLUGKIT_NAMESPACE_END

#endif
