#ifndef PLUGKIT_FILE_H
#define PLUGKIT_FILE_H

#include "export.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Context Context;
typedef struct Layer Layer;

typedef struct RawFrame {
  int link;
  const char *data;
  size_t length;
  size_t actualLength;
  int64_t tsSec;
  int64_t tsNsec;
  const Layer *root;
} RawFrame;

typedef enum FileStatus {
  FILE_STATUS_DONE,
  FILE_STATUS_ERROR,
  FILE_STATUS_UNSUPPORTED,
  FILE_STATUS_CANCELED
} FileStatus;

typedef bool(FileImporterCallback)(Context *ctx,
                                   const RawFrame *frames,
                                   size_t length,
                                   double progress);
typedef const RawFrame *(FileExporterCallback)(Context *ctx, size_t *length);

typedef FileStatus(FileImporterFunc)(Context *ctx,
                                     const char *filename,
                                     FileImporterCallback callback);
typedef FileStatus(FileExporterFunc)(Context *ctx,
                                     const char *filename,
                                     const RawFrame *frames,
                                     size_t length,
                                     FileExporterCallback callback);

typedef struct FileImporter {
  FileImporterFunc *func;
} FileImporter;

typedef struct FileExporter {
  FileExporterFunc *func;
} FileExporter;

PLUGKIT_NAMESPACE_END

#endif
