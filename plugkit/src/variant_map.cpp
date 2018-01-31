#include "variant_map.hpp"

namespace plugkit {

VariantMap::VariantMap() {}

VariantMap::VariantMap(const Map &map) : map(std::make_shared<Map>(map)) {}

const Variant &VariantMap::operator[](const std::string &key) const {
  auto it = map->find(key);
  if (it != map->end()) {
    return it->second;
  }
  static Variant null;
  return null;
}
} // namespace plugkit
