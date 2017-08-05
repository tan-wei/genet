#ifndef PLUGKIT_PROPERTY_HPP
#define PLUGKIT_PROPERTY_HPP

#include "property.h"
#include "variant.hpp"
#include "export.h"
#include "token.h"
#include <memory>
#include <vector>

namespace plugkit {

class PLUGKIT_EXPORT Property final {
public:
  Property(Token id, const Variant &value = Variant());
  ~Property();

  Token id() const;
  void setId(Token id);
  Range range() const;
  void setRange(const Range &range);
  Variant value() const;
  const Variant *valueRef() const;
  Variant *valueRef();
  void setValue(const Variant &value);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(Token id) const;
  void addProperty(const Property *prop);

private:
  Property(const Property &prop) = delete;
  Property &operator=(const Property &prop) = delete;

private:
  Token mId;
  Variant mValue;
  Range mRange;
  std::vector<const Property *> mChildren;
};
}

#endif
