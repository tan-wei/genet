#ifndef PLUGKIT_TYPES_H
#define PLUGKIT_TYPES_H

#include <chrono>
#include <memory>

namespace plugkit {
struct Layer;
struct Attr;
class Frame;
class FrameView;
struct Payload;
class Plugin;
struct SessionContext;

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

using Timestamp = std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::nanoseconds>;
} // namespace plugkit

#endif
