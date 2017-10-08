#ifndef PLUGKIT_SCRIPT_DISSECTOR_H
#define PLUGKIT_SCRIPT_DISSECTOR_H

#include "dissector.h"
#include <string>
#include <v8.h>

namespace plugkit {

class ScriptDissector {
public:
  ScriptDissector(const v8::Local<v8::Function> &ctor);
  static Dissector create(char *script);

private:
  v8::UniquePersistent<v8::Function> func;
};

} // namespace plugkit

#endif
