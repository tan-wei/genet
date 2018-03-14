#include <list>
#include <memory>
#include <set>
#include <string>

#ifndef PLUGKIT_STRING_POOL_HPP
#define PLUGKIT_STRING_POOL_HPP

namespace plugkit {

class StringPool {
public:
  const char *get(const char *str, size_t length);

private:
  struct View {
    const char *data;
    size_t length;
  };
  struct Comp {
    bool operator()(const View &lhs, const View &rhs);
  };
  std::set<View, Comp> set;
  std::list<std::shared_ptr<std::string>> holder;
};

} // namespace plugkit

#endif