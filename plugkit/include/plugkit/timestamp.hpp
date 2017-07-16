#ifndef PLUGKIT_TIMESTAMP_H
#define PLUGKIT_TIMESTAMP_H

#include <chrono>

namespace plugkit {

using Timestamp = std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::nanoseconds>;
}

#endif
