#ifndef PLUGKIT_FILTER_HPP
#define PLUGKIT_FILTER_HPP

#include <v8.h>
#include <functional>
#include <string>
#include <unordered_map>

namespace plugkit {

class FrameView;
using FrameViewConstPtr = std::shared_ptr<const FrameView>;

class Filter {
public:
  struct Result {
    using Value = v8::Local<v8::Value>;
    Result(Value value, Value parent = Value())
        : value(value), parent(parent) {}
    Value value;
    Value parent;
  };

public:
  Filter(const std::string &body);
  ~Filter();
  bool test(const FrameViewConstPtr &frame) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}
#endif
