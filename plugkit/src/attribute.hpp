#ifndef PLUGKIT_PROPERTY_HPP
#define PLUGKIT_PROPERTY_HPP

#include "attribute.h"
#include "variant.hpp"
#include "token.h"
#include <memory>

namespace plugkit {

struct Attr final {
public:
  Attr(Token id, const Variant &value = Variant(), Token type = Token());
  ~Attr();

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
  Attr(const Attr &prop) = delete;
  Attr &operator=(const Attr &prop) = delete;

private:
  Token mId = 0;
  Variant mValue;
  Range mRange = {0, 0};
  Token mType = 0;
};
}

#endif
