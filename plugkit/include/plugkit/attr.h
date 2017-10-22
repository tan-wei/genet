#ifndef PLUGKIT_attrERTY_H
#define PLUGKIT_attrERTY_H

#include "export.h"
#include "range.h"
#include "slice.h"
#include "token.h"
#include <stdbool.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Variant Variant;
typedef struct Attr Attr;

/// Return the ID of the attr.
PLUGKIT_EXPORT Token Attr_id(const Attr *attr);

/// Find the first child `Attr` with the given `id` and return it.
///
/// If no attribute is found, return `NULL`.
PLUGKIT_EXPORT const Attr *Attr_attr(const Attr *attr, Token id);

/// Return the range of the attr.
PLUGKIT_EXPORT Range Attr_range(const Attr *attr);

/// Set the range of the attr.
PLUGKIT_EXPORT void Attr_setRange(Attr *attr, Range range);

/// Return the type of the attr.
PLUGKIT_EXPORT Token Attr_type(const Attr *attr);

/// Set the type of the attr.
PLUGKIT_EXPORT void Attr_setType(Attr *attr, Token type);

/// Return the error of the attr.
PLUGKIT_EXPORT Token Attr_error(const Attr *attr);

/// Set the error of the attr.
PLUGKIT_EXPORT void Attr_setError(Attr *attr, Token error);

/// Return the value associated with the attr.
PLUGKIT_EXPORT const Variant *Attr_value(const Attr *attr);

/// Return the mutable value associated with the attr.
PLUGKIT_EXPORT Variant *Attr_valueRef(Attr *attr);

/// Return the value associated with the attr as `bool`.
PLUGKIT_EXPORT bool Attr_bool(const Attr *attr);

/// Set the value of the attr to the given `bool` value.
PLUGKIT_EXPORT void Attr_setBool(Attr *attr, bool value);

/// Return the value associated with the attr as `int32_t`.
PLUGKIT_EXPORT int32_t Attr_int32(const Attr *attr);

/// Set the value of the attr to the given `int32_t` value.
PLUGKIT_EXPORT void Attr_setInt32(Attr *attr, int32_t value);

/// Return the value associated with the attr as `uint32_t`.
PLUGKIT_EXPORT uint32_t Attr_uint32(const Attr *attr);

/// Set the value of the attr to the given `uint32_t` value.
PLUGKIT_EXPORT void Attr_setUint32(Attr *attr, uint32_t value);

/// Return the value associated with the attr as `double`.
PLUGKIT_EXPORT double Attr_double(const Attr *attr);

/// Set the value of the attr to the given `double` value.
PLUGKIT_EXPORT void Attr_setDouble(Attr *attr, double value);

/// Return the value associated with the attr as a null-terminated string.
PLUGKIT_EXPORT const char *Attr_string(const Attr *attr);

/// Set the value of the attr to the given string.
///
/// See [Variant_setString](diss-api-c.md#c-func-variant-setstring) for details.
PLUGKIT_EXPORT void Attr_setString(Attr *attr, const char *str);

/// Return the value associated with the attr as `Slice`.
PLUGKIT_EXPORT Slice Attr_slice(const Attr *attr);

/// Set the value of the attr to the given `Slice` value.
///
/// See [Variant_setSlice](diss-api-c.md#c-func-variant-setslice) for details.
PLUGKIT_EXPORT void Attr_setSlice(Attr *attr, Slice slice);

/// Returns an element of the array at `index`.
///
/// See [Variant_arrayValue](diss-api-c.md#c-func-variant-arrayvalue) for
/// details.
PLUGKIT_EXPORT const Variant *Attr_arrayValue(const Attr *attr, size_t index);

/// Returns a mutable element of the array at `index`.
///
/// See [Variant_arrayValueRef](diss-api-c.md#c-func-variant-arrayvalueref) for
/// details.
PLUGKIT_EXPORT Variant *Attr_arrayValueRef(Attr *attr, size_t index);

/// Returns an element of the map corresponded to `key`.
///
/// See [Variant_mapValue](diss-api-c.md#c-func-variant-mapvalue) for details.
PLUGKIT_EXPORT const Variant *
Attr_mapValue(const Attr *attr, const char *key, int length);

/// Returns a mutable element of the map corresponded to `key`.
///
/// See [Variant_mapValueRef](diss-api-c.md#c-func-variant-mapvalueref) for
/// details.
PLUGKIT_EXPORT Variant *
Attr_mapValueRef(Attr *attr, const char *key, int length);

PLUGKIT_NAMESPACE_END

#endif
