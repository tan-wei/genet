#include "plugkit_private.hpp"

namespace plugkit {

PlugKit &PlugKit::getInstance() { return PlugKitPrivate::getInstance(); }
}
