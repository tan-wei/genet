#ifndef PLUGKIT_H
#define PLUGKIT_H

#include "export.h"
#include <memory>

namespace plugkit {

class PLUGKIT_EXPORT PlugKit {
public:
  static PlugKit &getInstance();
};
} // namespace plugkit

#endif
