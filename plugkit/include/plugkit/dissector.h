#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "token.h"

extern "C" {

class Layer;
class Context;
class DissectionResult;

typedef void Worker;

struct Dissector {
  void (*analyze)(Worker *worker, Layer *layer, DissectionResult *result);
  Worker *(*createWorker)(const Context *ctx);
  Token layerHints[8];
};

void Dissector_init(Dissector *diss);
}

#endif
