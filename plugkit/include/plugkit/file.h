#ifndef PLUGKIT_CONTEXT_H
#define PLUGKIT_CONTEXT_H

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
  int64_t tsMsec;
  int64_t tsNsec;
  const Layer *root;
} RawFrame;

typedef bool(FileImporterCallback)(const RawFrame *frames,
                                   size_t length,
                                   double progress);
typedef bool(FileExporterCallback)(double progress);

typedef bool(FileImporter)(Context *ctx,
                           const char *filename,
                           FileImporterCallback callback);
typedef bool(FileExporter)(Context *ctx,
                           const char *filename,
                           const RawFrame *frames,
                           size_t length,
                           FileExporterCallback callback);

PLUGKIT_NAMESPACE_END

#endif
