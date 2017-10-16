#ifndef PLUGKIT_READER_H
#define PLUGKIT_READER_H

#include "export.h"
#include "range.h"
#include "slice.h"
#include "token.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Reader {
  Slice data;
  Range lastRange;
  Token lastError;
} Reader;

/// Clears Reader's state
PLUGKIT_EXPORT void Reader_reset(Reader *reader);
PLUGKIT_EXPORT Slice Reader_slice(Reader *reader, size_t begin, size_t end);
PLUGKIT_EXPORT Slice Reader_sliceAll(Reader *reader, size_t begin);

/// Read the next byte from `reader.data` and return it as `uint8_t`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint8_t Reader_getUint8(Reader *reader);

/// Read the next byte from `reader.data` and return it as `int8_t`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT int8_t Reader_getInt8(Reader *reader);

/// Read the next 2 bytes from `reader.data`
/// and return it as `uint16_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint16_t Reader_getUint16(Reader *reader, bool littleEndian);

/// Read the next 4 bytes from `reader.data`
/// and return it as `uint32_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint32_t Reader_getUint32(Reader *reader, bool littleEndian);

/// Read the next 8 bytes from `reader.data`
/// and return it as `uint64_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT uint64_t Reader_getUint64(Reader *reader, bool littleEndian);

/// Read the next 2 bytes from `reader.data`
/// and return it as `int16_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT int16_t Reader_getInt16(Reader *reader, bool littleEndian);

/// Read the next 4 bytes from `reader.data`
/// and return it as `int32_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT int32_t Reader_getInt32(Reader *reader, bool littleEndian);

/// Read the next 8 bytes from `reader.data`
/// and return it as `int64_t`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT int64_t Reader_getInt64(Reader *reader, bool littleEndian);

/// Read the next 4 bytes from `reader.data`
/// and return it as `float`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0.0f` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT float Reader_getFloat32(Reader *reader, bool littleEndian);

/// Read the next 8 bytes from `reader.data`
/// and return it as `float`.
/// Byte sequences are traeted as big-endian unless `littleEndian` is `true`.
///
/// If the length of the slice is not enougth,
/// return `0.0` and `reader.lastError` is set to `!out-of-bounds`
PLUGKIT_EXPORT double Reader_getFloat64(Reader *reader, bool littleEndian);

PLUGKIT_NAMESPACE_END

#endif
