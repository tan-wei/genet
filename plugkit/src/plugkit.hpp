#ifndef PLUGKIT_H
#define PLUGKIT_H

#include <memory>
#include "export.h"

namespace plugkit {

class PLUGKIT_EXPORT PlugKit {
public:
  static PlugKit &getInstance();
};
}

#endif
