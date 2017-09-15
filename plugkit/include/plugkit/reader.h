#ifndef PLUGKIT_READER_H
#define PLUGKIT_READER_H

#include "slice.h"
#include "range.h"
#include "error.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Reader {
  Slice slice;
  Range lastRange;
  Error lastError;
} Reader;

/// Clears Reader's state
PLUGKIT_EXPORT void Reader_reset(Reader *reader);
PLUGKIT_EXPORT Slice Reader_slice(Reader *reader, size_t offset, size_t length);
PLUGKIT_EXPORT Slice Reader_sliceAll(Reader *reader, size_t offset);

PLUGKIT_EXPORT uint8_t Reader_readUint8(Reader *reader);
PLUGKIT_EXPORT int8_t Reader_readInt8(Reader *reader);

PLUGKIT_EXPORT uint16_t Reader_readUint16BE(Reader *reader);
PLUGKIT_EXPORT uint32_t Reader_readUint32BE(Reader *reader);
PLUGKIT_EXPORT uint64_t Reader_readUint64BE(Reader *reader);

PLUGKIT_EXPORT int16_t Reader_readInt16BE(Reader *reader);
PLUGKIT_EXPORT int32_t Reader_readInt32BE(Reader *reader);
PLUGKIT_EXPORT int64_t Reader_readInt64BE(Reader *reader);

PLUGKIT_EXPORT float Reader_readFloat32BE(Reader *reader);
PLUGKIT_EXPORT double Reader_readFloat64BE(Reader *reader);

PLUGKIT_EXPORT uint16_t Reader_readUint16LE(Reader *reader);
PLUGKIT_EXPORT uint32_t Reader_readUint32LE(Reader *reader);
PLUGKIT_EXPORT uint64_t Reader_readUint64LE(Reader *reader);

PLUGKIT_EXPORT int16_t Reader_readInt16LE(Reader *reader);
PLUGKIT_EXPORT int32_t Reader_readInt32LE(Reader *reader);
PLUGKIT_EXPORT int64_t Reader_readInt64LE(Reader *reader);

PLUGKIT_EXPORT float Reader_readFloat32LE(Reader *reader);
PLUGKIT_EXPORT double Reader_readFloat64LE(Reader *reader);

PLUGKIT_NAMESPACE_END

#endif
