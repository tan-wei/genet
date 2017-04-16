#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include <memory>
#include <vector>
#include "variant.hpp"

namespace plugkit {

class Property;
using PropertyConstPtr = std::shared_ptr<const Property>;

class Property final {
public:
  Property();
  ~Property();
  Property(const Property &prop);
  Property &operator=(const Property &prop);

  std::string name() const;
  void setName(const std::string &name);
  std::string id() const;
  void setId(const std::string &id);
  std::string range() const;
  void setRange(const std::string &range);
  std::string summary() const;
  void setSummary(const std::string &summary);
  Variant value() const;
  void setValue(const Variant &value);

  const std::vector<PropertyConstPtr> &properties() const;
  PropertyConstPtr propertyFromId(const std::string &id) const;
  void addProperty(const PropertyConstPtr &prop);

  PropertyConstPtr deepCopy() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
