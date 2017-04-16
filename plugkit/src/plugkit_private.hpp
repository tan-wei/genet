#ifndef PLUGKIT_PRIVATE_H
#define PLUGKIT_PRIVATE_H

#include "plugkit.hpp"

namespace plugkit {

class Session;

class PlugKitPrivate final : public PlugKit {
public:
  static PlugKitPrivate &getInstance();
  PlugKitPrivate();

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
