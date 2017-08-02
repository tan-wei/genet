#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "token.h"

extern "C" {

namespace plugkit {

class Layer;

class Context;

typedef void Worker;

enum DissectorType { DISSECTOR_PACKET = 0, DISSECTOR_STREAM = 1 };

struct Dissector {
  void (*analyze)(Context *ctx, Worker *worker, Layer *layer);
  Worker *(*createWorker)(Context *ctx);
  bool (*expired)(Worker *worker, uint32_t elapsed);
  Token layerHints[8];
  DissectorType type;
};
}
}

#endif
