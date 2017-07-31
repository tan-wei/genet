#ifndef PLUGKIT_TYPES_H
#define PLUGKIT_TYPES_H

#include <chrono>
#include <memory>

namespace plugkit {

extern "C" {
struct Range {
  uint32_t begin;
  uint32_t end;
};
}

class Layer;
class Property;
class Frame;
class FrameView;
class Slice;
class Payload;
class Plugin;
struct SessionContext;

class Attribute;
using AttributeConstPtr = std::shared_ptr<const Attribute>;

class Chunk;
using ChunkConstPtr = std::shared_ptr<const Chunk>;

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

using Timestamp = std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::nanoseconds>;
}

#endif
