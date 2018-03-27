#include "config_map.hpp"

namespace plugkit {

ConfigMap::ConfigMap() {}

ConfigMap::ConfigMap(const Map &map) : map(std::make_shared<Map>(map)) {}

const Variant &ConfigMap::operator[](const std::string &key) const {
  auto it = map->find(key);
  if (it != map->end()) {
    return it->second;
  }
  static Variant null;
  return null;
}
} // namespace plugkit
