#ifndef PLUGKIT_FILE_PRIVATE_H
#define PLUGKIT_FILE_PRIVATE_H

#include "variant.hpp"

namespace plugkit {

struct Context;
struct Layer;

struct RawFrame {
  int link;
  const char *payload;
  size_t length;
  size_t actualLength;
  int64_t tsSec;
  int64_t tsNsec;
  const Layer *root;
  Variant data;
};

enum FileStatus { FILE_STATUS_DONE, FILE_STATUS_ERROR };

typedef bool(FileImporterCallback)(Context *ctx,
                                   size_t length,
                                   double progress);
typedef const RawFrame *(FileExporterCallback)(Context *ctx, size_t *length);

typedef bool(FileIsSupportedFunc)(Context *ctx, const char *filename);

typedef FileStatus(FileImporterFunc)(Context *ctx,
                                     const char *filename,
                                     RawFrame *frames,
                                     size_t capacity,
                                     FileImporterCallback callback);
typedef FileStatus(FileExporterFunc)(Context *ctx,
                                     const char *filename,
                                     FileExporterCallback callback);

struct FileImporter {
  FileIsSupportedFunc *isSupported;
  FileImporterFunc *start;
};

struct FileExporter {
  FileIsSupportedFunc *isSupported;
  FileExporterFunc *start;
};

} // namespace plugkit

#endif
