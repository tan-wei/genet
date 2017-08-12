#ifndef PLUGKIT_DISSECTOR_HPP
#define PLUGKIT_DISSECTOR_HPP

#include "dissector.h"
#include <vector>

namespace plugkit {

struct Dissector {
  AnalyzerFunc *analyze;
  WokerFactoryFunc *createWorker;
  ExpiryFunc *expired;
  std::vector<Token> layerHints;
  DissectorType type;
};
}

#endif
