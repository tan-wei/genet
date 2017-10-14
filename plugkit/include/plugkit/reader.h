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

PLUGKIT_EXPORT uint8_t Reader_getUint8(Reader *reader);
PLUGKIT_EXPORT int8_t Reader_getInt8(Reader *reader);

PLUGKIT_EXPORT uint16_t Reader_getUint16(Reader *reader, bool littleEndian);
PLUGKIT_EXPORT uint32_t Reader_getUint32(Reader *reader, bool littleEndian);
PLUGKIT_EXPORT uint64_t Reader_getUint64(Reader *reader, bool littleEndian);

PLUGKIT_EXPORT int16_t Reader_getInt16(Reader *reader, bool littleEndian);
PLUGKIT_EXPORT int32_t Reader_getInt32(Reader *reader, bool littleEndian);
PLUGKIT_EXPORT int64_t Reader_getInt64(Reader *reader, bool littleEndian);

PLUGKIT_EXPORT float Reader_getFloat32(Reader *reader, bool littleEndian);
PLUGKIT_EXPORT double Reader_getFloat64(Reader *reader, bool littleEndian);

PLUGKIT_NAMESPACE_END

#endif
