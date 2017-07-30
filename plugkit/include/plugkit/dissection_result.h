#ifndef PLUGKIT_DISSECTION_RESULT_H
#define PLUGKIT_DISSECTION_RESULT_H

#include "token.h"

extern "C" {

namespace plugkit {
class Layer;

struct DissectionResult {
  class Layer *child;
  char streamIdentifier[256];
};
}
}

#endif
