#ifndef PLUGKIT_PROPERTY_HPP
#define PLUGKIT_PROPERTY_HPP

#include "range.hpp"
#include "token.hpp"
#include "variant.hpp"
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
  Attr(const Attr &attr) = delete;
  Attr &operator=(const Attr &attr) = delete;

private:
  Token mId = Token_null();
  Token mType = Token_null();
  Variant mValue;
  union Data {
    Range range;
    void *ext;
  } mData;
};

} // namespace plugkit

#endif
