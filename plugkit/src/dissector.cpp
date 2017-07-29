#include "dissector.hpp"
#include "dissector.h"
#include <chrono>

namespace plugkit {

Dissector::~Dissector() {}

Dissector::Worker::~Worker() {}

DissectorFactory::~DissectorFactory() {}
}
