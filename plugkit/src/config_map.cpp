#include "config_map.hpp"

namespace plugkit {

ConfigMap::ConfigMap() : map(std::make_shared<Map>()) {}

ConfigMap::ConfigMap(const Map &map) : map(std::make_shared<Map>(map)) {}

const std::string &ConfigMap::operator[](const std::string &key) const {
  auto it = map->find(key);
  if (it != map->end()) {
    return it->second;
  }
  static const std::string null;
  return null;
}
} // namespace plugkit
