#ifndef GENET_NODE_FILTER_H
#define GENET_NODE_FILTER_H

#include "script.hpp"

struct Filter;
struct Frame;

namespace genet_node {
class FilterIsolate : public Script {
public:
  FilterIsolate(const std::string &data);
  ~FilterIsolate();
  bool test(const Frame *);
  static Filter *createFilter(const char *data, size_t length);

private:
  FilterIsolate() = delete;
  FilterIsolate(const FilterIsolate &) = delete;
  FilterIsolate &operator=(const FilterIsolate &) = delete;

private:
  v8::UniquePersistent<v8::Function> testFunc;
};
} // namespace genet_node

#endif