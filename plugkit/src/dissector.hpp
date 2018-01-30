#ifndef PLUGKIT_DISSECTOR_PRIVATE_H
#define PLUGKIT_DISSECTOR_PRIVATE_H

#include "token.hpp"

namespace plugkit {

struct Context;
struct Dissector;
struct Layer;

struct Worker {
  void *data;
};

typedef void(IntializeFunc)(Context *ctx, Dissector *);
typedef void(TerminateFunc)(Context *ctx, Dissector *);
typedef Worker(CreateWorkerFunc)(Context *ctx, const Dissector *);
typedef void(DestroyWorkerFunc)(Context *ctx, const Dissector *, Worker);
typedef void(AnalyzeFunc)(Context *ctx, const Dissector *, Worker, Layer *);

struct Dissector {
  IntializeFunc *initialize;
  TerminateFunc *terminate;
  CreateWorkerFunc *createWorker;
  DestroyWorkerFunc *destroyWorker;
  AnalyzeFunc *analyze;
  Token layerHints[8];
  void *data;
};

} // namespace plugkit

#endif
