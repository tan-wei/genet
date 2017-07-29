#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "token.h"

extern "C" {

namespace plugkit {
class Layer;
}
using namespace plugkit;

class Context;
struct DissectionResult;

typedef void Worker;

enum DissectorType { DISSECTOR_PACKET = 0, DISSECTOR_STREAM = 1 };

struct XDissector {
  void (*analyze)(Worker *worker, Layer *layer, DissectionResult *result);
  Worker *(*createWorker)(Context *ctx);
  bool *(*expired)(int elapsed);
  Token layerHints[8];
  DissectorType type;
};
}

#endif
