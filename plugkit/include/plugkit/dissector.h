#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "export.h"
#include "token.h"
#include <stdbool.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Layer Layer;
typedef struct Context Context;
typedef struct Dissector Dissector;

typedef struct Worker {
  void *data;
} Worker;

typedef void(IntializeFunc)(Context *ctx, Dissector *);
typedef void(TerminateFunc)(Context *ctx, Dissector *);
typedef Worker(CreateWorkerFunc)(Context *ctx, const Dissector *);
typedef void(DestroyWorkerFunc)(Context *ctx, const Dissector *, Worker);
typedef void(AnalyzeFunc)(Context *ctx, const Dissector *, Worker, Layer *);

PLUGKIT_NAMESPACE_END

#endif
