#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "token.h"

extern "C" {

class Layer;
class Context;
class DissectionResult;

typedef void Worker;

enum DissectorType { DISSECTOR_PACKET = 0, DISSECTOR_STREAM = 1 };

struct Dissector {
  void (*analyze)(Worker *worker, Layer *layer, DissectionResult *result);
  Worker *(*createWorker)(const Context *ctx);
  bool *(*expired)(int elapsed);
  Token layerHints[8];
  DissectorType type;
};
}

#endif
