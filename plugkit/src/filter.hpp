#ifndef PLUGKIT_FILTER_HPP
#define PLUGKIT_FILTER_HPP

#include <string>
#include <v8.h>

namespace plugkit {

class FrameView;

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
  bool test(const FrameView *frame) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}
#endif
