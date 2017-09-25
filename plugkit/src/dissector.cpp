#include "dissector.hpp"

namespace plugkit {

Dissector *Dissector_create(DissectorType type) {
  Dissector *diss = new Dissector();
  diss->type = type;
  return diss;
}
void Dissector_setAnalyzer(Dissector *diss, AnalyzerFunc *func) {
  diss->analyze = func;
}
void Dissector_setWorkerFactory(Dissector *diss, WokerFactoryAllocFunc *alloc,
                                WokerFactoryDeallocFunc *dealloc) {
  diss->createWorker = alloc;
  diss->destroyWorker = dealloc;
}
void Dissector_addLayerHint(Dissector *diss, Token hint) {
  diss->layerHints.push_back(hint);
}
} // namespace plugkit
