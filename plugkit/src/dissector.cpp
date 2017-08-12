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
void Dissector_setWorkerFactory(Dissector *diss, WokerFactoryFunc *func) {
  diss->createWorker = func;
}
void Dissector_setExpiry(Dissector *diss, ExpiryFunc *func) {
  diss->expired = func;
}
void Dissector_addLayerHint(Dissector *diss, Token hint) {
  diss->layerHints.push_back(hint);
}
}
