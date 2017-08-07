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
  Range range() const;
  void setRange(const Range &range);
  Variant value() const;
  const Variant *valueRef() const;
  Variant *valueRef();
  void setValue(const Variant &value);
  Token type() const;
  void setType(Token type);

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(Token id) const;
  void addProperty(const Property *prop);

private:
  Property(const Property &prop) = delete;
  Property &operator=(const Property &prop) = delete;

private:
  Token mId = 0;
  Variant mValue;
  Range mRange = {0, 0};
  Token mType = 0;
  std::vector<const Property *> mChildren;
};
}

#endif
