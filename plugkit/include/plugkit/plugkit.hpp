#ifndef PLUGKIT_H
#define PLUGKIT_H

#include <memory>

namespace plugkit {

class Plugin;

class PlugKit {
public:
  static PlugKit &getInstance();
};
}

#endif
