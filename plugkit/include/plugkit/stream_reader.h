#ifndef PLUGKIT_STREAM_READER_H
#define PLUGKIT_STREAM_READER_H

#include "export.h"
#include "range.h"
#include "slice.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Payload Payload;
typedef struct StreamReader StreamReader;

/// Allocate a new StreamReader instance.
PLUGKIT_EXPORT StreamReader *StreamReader_create();

/// Deallocate a StreamReader instance.
PLUGKIT_EXPORT void StreamReader_destroy(StreamReader *reader);

/// Return the length of the stream.
PLUGKIT_EXPORT size_t StreamReader_length(const StreamReader *reader);

/// Add `slice` to the stream.
PLUGKIT_EXPORT void StreamReader_addSlice(StreamReader *reader, Slice slice);

/// Add all slices in `payload` to the stream.
PLUGKIT_EXPORT void StreamReader_addPayload(StreamReader *reader,
                                            const Payload *payload);

/// Find the first occurrence of the given byte sequence in the stream.
/// The first `offset` bytes in the stream are skipped.
///
/// Return the position of the found byte sequence,
/// or `RANGE_NPOS` if no such byte sequence is found.
PLUGKIT_EXPORT size_t StreamReader_search(StreamReader *reader,
                                          const char *data,
                                          size_t length,
                                          size_t offset);

/// Read data from the stream up to `length` bytes.
/// The first `offset` bytes in the stream are skipped.
///
/// The capacity of `buffer` must be at least `length` bytes.
///
/// If all slices are contiguous in the memory, just return a concatenated slice.
/// Otherwise copy all slices into `buffer` and return a slice of `buffer`.
PLUGKIT_EXPORT Slice StreamReader_read(StreamReader *reader,
                                       char *buffer,
                                       size_t length,
                                       size_t offset);

PLUGKIT_NAMESPACE_END

#endif
