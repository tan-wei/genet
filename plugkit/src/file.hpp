#ifndef PLUGKIT_FILE_PRIVATE_H
#define PLUGKIT_FILE_PRIVATE_H

namespace plugkit {

struct Context;
struct Layer;

typedef struct RawFrame {
  int link;
  const char *data;
  size_t length;
  size_t actualLength;
  int64_t tsSec;
  int64_t tsNsec;
  const Layer *root;
} RawFrame;

enum FileStatus {
  FILE_STATUS_DONE,
  FILE_STATUS_ERROR,
  FILE_STATUS_UNSUPPORTED
};

typedef bool(FileImporterCallback)(Context *ctx,
                                   size_t length,
                                   double progress);
typedef const RawFrame *(FileExporterCallback)(Context *ctx, size_t *length);

typedef FileStatus(FileImporterFunc)(Context *ctx,
                                     const char *filename,
                                     RawFrame *frames,
                                     size_t capacity,
                                     FileImporterCallback callback);
typedef FileStatus(FileExporterFunc)(Context *ctx,
                                     const char *filename,
                                     FileExporterCallback callback);

typedef struct FileImporter {
  FileImporterFunc *func;
} FileImporter;

typedef struct FileExporter {
  FileExporterFunc *func;
} FileExporter;

} // namespace plugkit

#endif
