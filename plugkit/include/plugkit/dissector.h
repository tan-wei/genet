#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "token.h"
#include <stdbool.h>

PLUGKIT_NAMESPACE_BEGIN

struct Layer;
typedef struct Layer Layer;

struct Context;
typedef struct Context Context;

typedef void Worker;

typedef enum DissectorType {
  DISSECTOR_PACKET = 0,
  DISSECTOR_STREAM = 1
} DissectorType;

struct Dissector {
  void (*analyze)(Context *ctx, Worker *worker, Layer *layer);
  Worker *(*createWorker)(Context *ctx);
  bool (*expired)(Worker *worker, uint32_t elapsed);
  Token layerHints[8];
  DissectorType type;
};

PLUGKIT_NAMESPACE_END

#endif
