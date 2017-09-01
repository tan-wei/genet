#ifndef PLUGKIT_FILTER_HPP
#define PLUGKIT_FILTER_HPP

#include <string>
#include <v8.h>

namespace plugkit {

class FrameView;

class Filter final {
public:
  struct Result final {
    using Value = v8::Local<v8::Value>;
    Result(Value value, Value parent = Value())
        : value(value), parent(parent) {}
    Value value;
    Value parent;
  };

public:
  Filter(const std::string &body);
  ~Filter();
  void test(char *results, const FrameView **begin, size_t size) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}
#endif
