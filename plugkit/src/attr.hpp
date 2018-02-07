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
  Token mId = 0;
  Variant mValue;
  Range mRange = {0, 0};
  Token mType = 0;
};

extern "C" {
/// Return the ID of the attr.
Token Attr_id(const Attr *attr);

/// Return the range of the attr.
Range Attr_range(const Attr *attr);

/// Set the range of the attr.
void Attr_setRange(Attr *attr, Range range);

/// Return the type of the attr.
Token Attr_type(const Attr *attr);

/// Set the type of the attr.
void Attr_setType(Attr *attr, Token type);

/// Return the value associated with the attr.
const Variant *Attr_value(const Attr *attr);

/// Return the mutable value associated with the attr.
Variant *Attr_valueRef(Attr *attr);

/// Return the value associated with the attr as `bool`.
bool Attr_bool(const Attr *attr);

/// Set the value of the attr to the given `bool` value.
void Attr_setBool(Attr *attr, bool value);

/// Return the value associated with the attr as `int64_t`.
int64_t Attr_int64(const Attr *attr);

/// Set the value of the attr to the given `int64_t` value.
void Attr_setInt64(Attr *attr, int64_t value);

/// Return the value associated with the attr as `uint64_t`.
uint64_t Attr_uint64(const Attr *attr);

/// Set the value of the attr to the given `uint64_t` value.
void Attr_setUint64(Attr *attr, uint64_t value);

/// Return the value associated with the attr as `double`.
double Attr_double(const Attr *attr);

/// Set the value of the attr to the given `double` value.
void Attr_setDouble(Attr *attr, double value);

/// Return the value associated with the attr as a null-terminated string.
const char *Attr_string(const Attr *attr);

/// Set the value of the attr to the given string.
///
/// See [Variant_setString](diss-api-c.md#c-func-variant-setstring) for details.
void Attr_setString(Attr *attr, const char *str, int length);

/// Return the value associated with the attr as `Slice`.
Slice Attr_slice(const Attr *attr);

/// Set the value of the attr to the given `Slice` value.
///
/// See [Variant_setSlice](diss-api-c.md#c-func-variant-setslice) for details.
void Attr_setSlice(Attr *attr, Slice slice);

/// Returns an element of the array at `index`.
///
/// See [Variant_arrayValue](diss-api-c.md#c-func-variant-arrayvalue) for
/// details.
const Variant *Attr_arrayValue(const Attr *attr, size_t index);

/// Returns a mutable element of the array at `index`.
///
/// See [Variant_arrayValueRef](diss-api-c.md#c-func-variant-arrayvalueref) for
/// details.
Variant *Attr_arrayValueRef(Attr *attr, size_t index);

/// Returns an element of the map corresponded to `key`.
///
/// See [Variant_mapValue](diss-api-c.md#c-func-variant-mapvalue) for details.
const Variant *Attr_mapValue(const Attr *attr, const char *key, int length);

/// Returns a mutable element of the map corresponded to `key`.
///
/// See [Variant_mapValueRef](diss-api-c.md#c-func-variant-mapvalueref) for
/// details.
Variant *Attr_mapValueRef(Attr *attr, const char *key, int length);
}

} // namespace plugkit

#endif
