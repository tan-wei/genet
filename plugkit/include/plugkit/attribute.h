#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include "token.h"
#include "range.h"
#include "slice.h"
#include "export.h"
#include <stdbool.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Variant Variant;
typedef struct Attr Attr;

/// Gets id
PLUGKIT_EXPORT Token Attr_id(const Attr *prop);

/// Finds the first child attribute with the given id and returns it.
///
/// If no attribute is found, returns nullptr.
PLUGKIT_EXPORT const Attr *Attr_attr(const Attr *prop, Token id);

/// Gets range
PLUGKIT_EXPORT Range Attr_range(const Attr *prop);

/// Sets range
PLUGKIT_EXPORT void Attr_setRange(Attr *prop, Range range);

/// Gets type
PLUGKIT_EXPORT Token Attr_type(const Attr *prop);

/// Sets type
PLUGKIT_EXPORT void Attr_setType(Attr *prop, Token type);

/// Gets value as a const pointer
PLUGKIT_EXPORT const Variant *Attr_value(const Attr *prop);

/// Gets value as a mutable pointer
PLUGKIT_EXPORT Variant *Attr_valueRef(Attr *prop);

/// Gets a boolean value
PLUGKIT_EXPORT bool Attr_bool(const Attr *prop);

/// Sets a boolean value
PLUGKIT_EXPORT void Attr_setBool(Attr *prop, bool value);

/// Gets an integer value
PLUGKIT_EXPORT int32_t Attr_int32(const Attr *prop);

/// Sets an integer value
PLUGKIT_EXPORT void Attr_setInt32(Attr *prop, int32_t value);

/// Gets an integer value
PLUGKIT_EXPORT int64_t Attr_int64(const Attr *prop);

/// Sets an integer value
PLUGKIT_EXPORT void Attr_setInt64(Attr *prop, int64_t value);

/// Gets an unsigned integer value
PLUGKIT_EXPORT uint32_t Attr_uint32(const Attr *prop);

/// Gets an unsigned integer value
PLUGKIT_EXPORT void Attr_setUint32(Attr *prop, uint32_t value);

/// Gets an unsigned integer value
PLUGKIT_EXPORT uint64_t Attr_uint64(const Attr *prop);

/// Gets an unsigned integer value
PLUGKIT_EXPORT void Attr_setUint64(Attr *prop, uint64_t value);

/// Gets a floating number value
PLUGKIT_EXPORT double Attr_double(const Attr *prop);

/// Sets a floating number value
PLUGKIT_EXPORT void Attr_setDouble(Attr *prop, double value);

/// Gets a string value
PLUGKIT_EXPORT const char *Attr_string(const Attr *prop);

/// Sets a string value
PLUGKIT_EXPORT void Attr_setString(Attr *prop, const char *str);

/// Sets a slice value
PLUGKIT_EXPORT Slice Attr_slice(const Attr *prop);

/// Sets a slice value
PLUGKIT_EXPORT void Attr_setSlice(Attr *prop, Slice slice);

PLUGKIT_EXPORT const Variant *Attr_arrayValue(const Attr *prop, size_t index);

/// Gets a value associated with a given key
PLUGKIT_EXPORT const Variant *Attr_mapValue(const Attr *prop, const char *key,
                                            int length);

/// Gets a mutable value associated with a given key
PLUGKIT_EXPORT Variant *Attr_mapValueRef(Attr *prop, const char *key,
                                         int length);

PLUGKIT_NAMESPACE_END

#endif
