#include "string_pool.hpp"
#include <cstring>

namespace plugkit {
const char *StringPool::get(const char *str, size_t length) {
  if (length == 0) {
    return "";
  }
  View view{str, length};
  auto it = set.lower_bound(view);
  if (it != set.end() && strncmp(view.data, it->data, length) == 0) {
    return it->data;
  }
  auto ptr = std::make_shared<std::string>(str, length);
  holder.push_back(ptr);
  view.data = ptr->c_str();
  set.insert(view);
  return view.data;
}

bool StringPool::Comp::operator()(const View &lhs, const View &rhs) {
  return strncmp(lhs.data, rhs.data, std::min(lhs.length, rhs.length)) < 0;
}
} // namespace plugkit
