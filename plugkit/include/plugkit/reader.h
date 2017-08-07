#ifndef PLUGKIT_READER_H
#define PLUGKIT_READER_H

#include "view.h"
#include "range.h"
#include "error.h"

extern "C" {

namespace plugkit {

struct Reader {
  View view;
  Range lastRange;
  Error lastError;
};

/// Clears Reader's state
void Reader_reset(Reader *reader);
View Reader_slice(Reader *reader, size_t offset, size_t length);
View Reader_sliceAll(Reader *reader, size_t offset);

uint8_t Reader_readUint8(Reader *reader);
int8_t Reader_readInt8(Reader *reader);

uint16_t Reader_readUint16BE(Reader *reader);
uint32_t Reader_readUint32BE(Reader *reader);
uint64_t Reader_readUint64BE(Reader *reader);

int16_t Reader_readInt16BE(Reader *reader);
int32_t Reader_readInt32BE(Reader *reader);
int64_t Reader_readInt64BE(Reader *reader);

float Reader_readFloat32BE(Reader *reader);
double Reader_readFloat64BE(Reader *reader);

uint16_t Reader_readUint16LE(Reader *reader);
uint32_t Reader_readUint32LE(Reader *reader);
uint64_t Reader_readUint64LE(Reader *reader);

int16_t Reader_readInt16LE(Reader *reader);
int32_t Reader_readInt32LE(Reader *reader);
int64_t Reader_readInt64LE(Reader *reader);

float Reader_readFloat32LE(Reader *reader);
double Reader_readFloat64LE(Reader *reader);
}
}

#endif
