#include "dissector.hpp"
#include <chrono>

namespace plugkit {

Dissector::~Dissector() {}

Dissector::Worker::~Worker() {}

const char *Dissector::Worker::streamId() { return nullptr; }

size_t Dissector::streamIdLength() const { return 0; }

DissectorFactory::~DissectorFactory() {}
}
