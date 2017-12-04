#ifndef PLUGKIT_VARIANT_MAP_HPP
#define PLUGKIT_VARIANT_MAP_HPP

#include "variant.hpp"
#include <unordered_map>

namespace plugkit {

class VariantMap final {
public:
  Variant &operator[](const std::string &key);
  const Variant &operator[](const std::string &key) const;

private:
  std::unordered_map<std::string, Variant> map;
};
} // namespace plugkit

#endif
