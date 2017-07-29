#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include "variant.hpp"
#include "export.hpp"
#include "miniid.hpp"
#include <memory>
#include <vector>

namespace plugkit {

class PLUGKIT_EXPORT Property final {
public:
  Property(miniid id, const Variant &value = Variant());
  ~Property();

  miniid id() const;
  void setId(miniid id);
  std::pair<uint32_t, uint32_t> range() const;
  void setRange(const std::pair<uint32_t, uint32_t> &range);
  std::string summary() const;
  void setSummary(const std::string &summary);
  Variant value() const;
  void setValue(const Variant &value);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(miniid id) const;
  void addProperty(const Property *prop);

private:
  Property(const Property &prop) = delete;
  Property &operator=(const Property &prop) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
