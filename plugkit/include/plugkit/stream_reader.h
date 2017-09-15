#ifndef PLUGKIT_STREAM_READER_H
#define PLUGKIT_STREAM_READER_H

#include "slice.h"
#include "range.h"
#include "error.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Payload Payload;
typedef struct StreamReader StreamReader;

PLUGKIT_EXPORT StreamReader *StreamReader_create();
PLUGKIT_EXPORT void StreamReader_destroy(StreamReader *reader);
PLUGKIT_EXPORT size_t StreamReader_length(const StreamReader *reader);
PLUGKIT_EXPORT void StreamReader_addSlice(StreamReader *reader, Slice slice);
PLUGKIT_EXPORT void StreamReader_addPayload(StreamReader *reader,
                                            const Payload *payload);
PLUGKIT_EXPORT Range StreamReader_search(StreamReader *reader, const char *data,
                                         size_t length, size_t offset);
PLUGKIT_EXPORT Slice StreamReader_read(StreamReader *reader, char *buffer,
                                       size_t length, size_t offset);

PLUGKIT_NAMESPACE_END

#endif
