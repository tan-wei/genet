/// @file
/// Binary sequence
#ifndef PLUGKIT_SLICE_H
#define PLUGKIT_SLICE_H

#include "token.hpp"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace plugkit {

struct Slice {
  const char *data;
  size_t length;
};

/// Return a `Slice` cropped by `begin` and `end` indices.
Slice Slice_slice(Slice slice, size_t begin, size_t end);

/// Return a `Slice` offset by `begin` bytes.
Slice Slice_sliceAll(Slice slice, size_t offset);

/// Read a byte from the `offset` index of the slice
/// and return it as `uint8_t`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
uint8_t Slice_getUint8(Slice slice, size_t offset, Token *err);

/// Read a byte from the `offset` index of the slice
/// and return it as `int8_t`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
int8_t Slice_getInt8(Slice slice, size_t offset, Token *err);

/// Read 2 bytes from the `offset` index of the slice
/// and return it as `uint16_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
uint16_t
Slice_getUint16(Slice slice, size_t offset, bool littleEndian, Token *err);

/// Read 4 bytes from the `offset` index of the slice
/// and return it as `uint32_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
uint32_t
Slice_getUint32(Slice slice, size_t offset, bool littleEndian, Token *err);

/// Read 8 bytes from the `offset` index of the slice
/// and return it as `uint64_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
uint64_t
Slice_getUint64(Slice slice, size_t offset, bool littleEndian, Token *err);

/// Read 2 bytes from the `offset` index of the slice
/// and return it as `int16_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
int16_t
Slice_getInt16(Slice slice, size_t offset, bool littleEndian, Token *err);

/// Read 4 bytes from the `offset` index of the slice
/// and return it as `int32_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
int32_t
Slice_getInt32(Slice slice, size_t offset, bool littleEndian, Token *err);

/// Read 8 bytes from the `offset` index of the slice
/// and return it as `int64_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0` and `err` is set to `!out-of-bounds`
int64_t
Slice_getInt64(Slice slice, size_t offset, bool littleEndian, Token *err);

/// Read 4 bytes from the `offset` index of the slice
/// and return it as `float`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0.0f` and `err` is set to `!out-of-bounds`
float Slice_getFloat32(Slice slice,
                       size_t offset,
                       bool littleEndian,
                       Token *err);

/// Read 8 bytes from the `offset` index of the slice
/// and return it as `double`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enough,
/// return `0.0` and `err` is set to `!out-of-bounds`
double
Slice_getFloat64(Slice slice, size_t offset, bool littleEndian, Token *err);

} // namespace plugkit

#endif
