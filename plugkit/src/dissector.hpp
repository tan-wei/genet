#ifndef PLUGKIT_DISSECTOR_PRIVATE_H
#define PLUGKIT_DISSECTOR_PRIVATE_H

#include "layer.hpp"
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
typedef uint32_t(ExamineFunc)(Context *ctx,
                              const Dissector *,
                              Worker,
                              const Layer *);
typedef void(AnalyzeFunc)(Context *ctx, const Dissector *, Worker, Layer *);

struct Dissector {
  IntializeFunc *initialize = nullptr;
  TerminateFunc *terminate = nullptr;
  CreateWorkerFunc *createWorker = nullptr;
  DestroyWorkerFunc *destroyWorker = nullptr;
  ExamineFunc *examine = nullptr;
  AnalyzeFunc *analyze = nullptr;
  Token layerHints[8] = {0};
  void *data = nullptr;
};

} // namespace plugkit

#endif
