#include "dissector.hpp"
#include <chrono>

namespace plugkit {

Dissector::~Dissector() {}

Dissector::Worker::~Worker() {}

DissectorFactory::~DissectorFactory() {}
}
