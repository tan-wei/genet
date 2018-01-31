#ifndef PLUGKIT_VARIANT_MAP_HPP
#define PLUGKIT_VARIANT_MAP_HPP

#include "variant.hpp"
#include <memory>
#include <unordered_map>

namespace plugkit {

class VariantMap final {
public:
  using Map = std::unordered_map<std::string, Variant>;

public:
  VariantMap();
  VariantMap(const Map &map);
  const Variant &operator[](const std::string &key) const;

private:
  std::shared_ptr<Map> map;
};
} // namespace plugkit

#endif
