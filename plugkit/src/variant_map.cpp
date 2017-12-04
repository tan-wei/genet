#include "variant_map.hpp"

namespace plugkit {

Variant &VariantMap::operator[](const std::string &key) { return map[key]; }

const Variant &VariantMap::operator[](const std::string &key) const {
  auto it = map.find(key);
  if (it != map.end()) {
    return it->second;
  }
  static Variant null;
  return null;
}
} // namespace plugkit
