#ifndef PLUGKIT_PROPERTY_HPP
#define PLUGKIT_PROPERTY_HPP

#include "property.h"
#include "variant.hpp"
#include "export.h"
#include "token.h"
#include <memory>

namespace plugkit {

struct PLUGKIT_EXPORT Property final {
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

private:
  Property(const Property &prop) = delete;
  Property &operator=(const Property &prop) = delete;

private:
  Token mId = 0;
  Variant mValue;
  Range mRange = {0, 0};
  Token mType = 0;
};
}

#endif
