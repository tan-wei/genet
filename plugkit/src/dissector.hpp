#ifndef PLUGKIT_DISSECTOR_PRIVATE_H
#define PLUGKIT_DISSECTOR_PRIVATE_H

#include "dissector.h"

namespace plugkit {

typedef struct Dissector {
  IntializeFunc *initialize;
  TerminateFunc *terminate;
  CreateWorkerFunc *createWorker;
  DestroyWorkerFunc *destroyWorker;
  AnalyzeFunc *analyze;
  Token layerHints[8];
  void *data;
} Dissector;

} // namespace plugkit

#endif
