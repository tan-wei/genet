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
  const Variant &value() const;
  Variant &value();
  Token type() const;
  void setType(Token type);

private:
  Attr(const Attr &attr) = delete;
  Attr &operator=(const Attr &attr) = delete;

private:
  Token mId = Token_null();
  Token mType = Token_null();
  Variant mValue;
  Range mRange;
};

} // namespace plugkit

#endif
