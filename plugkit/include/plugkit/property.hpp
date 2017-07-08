#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include "variant.hpp"
#include "export.hpp"
#include "strid.hpp"
#include <memory>
#include <vector>

namespace plugkit {

class Property;

class PLUGKIT_EXPORT Property final {
public:
  Property();
  Property(strid id, const Variant &value = Variant());
  Property(Property &&prop);
  ~Property();

  strid id() const;
  void setId(strid id);
  std::pair<uint32_t, uint32_t> range() const;
  void setRange(const std::pair<uint32_t, uint32_t> &range);
  std::string summary() const;
  void setSummary(const std::string &summary);
  std::string error() const;
  void setError(const std::string &error);
  Variant value() const;
  void setValue(const Variant &value);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(strid id) const;
  void addProperty(const Property *prop);
  void addProperty(Property &&prop);

private:
  Property(const Property &prop) = delete;
  Property &operator=(const Property &prop) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
