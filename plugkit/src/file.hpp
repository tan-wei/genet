#ifndef PLUGKIT_FILE_PRIVATE_H
#define PLUGKIT_FILE_PRIVATE_H

#include "file.h"

namespace plugkit {

typedef struct FileImporter {
  FileImporterFunc *func;
} FileImporter;

typedef struct FileExporter {
  FileExporterFunc *func;
} FileExporter;

} // namespace plugkit

#endif
