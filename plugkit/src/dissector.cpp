#include "dissector.hpp"
#include <chrono>

namespace plugkit {

Dissector::~Dissector() {}

Dissector::Worker::~Worker() {}

DissectorFactory::~DissectorFactory() {}

std::vector<DissectorFactory::TestData> DissectorFactory::testData() const {
  return std::vector<TestData>();
}
}
