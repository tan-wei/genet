#ifndef PLUGKI_CONFIG_MAP_HPP
#define PLUGKI_CONFIG_MAP_HPP

#include <memory>
#include <string>
#include <unordered_map>

namespace plugkit {

class ConfigMap final {
public:
  using Map = std::unordered_map<std::string, std::string>;

public:
  ConfigMap();
  ConfigMap(const Map &map);
  const std::string &operator[](const std::string &key) const;

private:
  std::shared_ptr<Map> map;
};
} // namespace plugkit

#endif
