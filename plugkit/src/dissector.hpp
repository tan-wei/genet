#ifndef PLUGKIT_DISSECTOR_HPP
#define PLUGKIT_DISSECTOR_HPP

#include "dissector.h"
#include <vector>

namespace plugkit {

struct Context;

struct Dissector {
  AnalyzerFunc *analyze;
  WokerFactoryAllocFunc *createWorker;
  WokerFactoryDeallocFunc *destroyWorker;
  std::vector<Token> layerHints;
  DissectorType type;
  Context *context;
};
} // namespace plugkit

#endif
