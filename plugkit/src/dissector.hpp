#ifndef PLUGKIT_DISSECTOR_HPP
#define PLUGKIT_DISSECTOR_HPP

#include "dissector.h"
#include <vector>

namespace plugkit {

struct Dissector {
  AnalyzerFunc *analyze;
  WokerFactoryAllocFunc *createWorker;
  WokerFactoryDeallocFunc *destroyWorker;
  std::vector<Token> layerHints;
  DissectorType type;
};
} // namespace plugkit

#endif
