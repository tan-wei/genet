#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include "token.h"
#include "range.h"
#include "slice.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef struct Variant Variant;
typedef struct Property Property;

/// Gets id
PLUGKIT_EXPORT Token Property_id(const Property *prop);

/// Finds the first child property with the given id and returns it.
///
/// If no property is found, returns nullptr.
PLUGKIT_EXPORT const Property *Property_propertyFromId(const Property *prop,
                                                       Token id);

/// Gets range
PLUGKIT_EXPORT Range Property_range(const Property *prop);

/// Sets range
PLUGKIT_EXPORT void Property_setRange(Property *prop, Range range);

/// Gets type
PLUGKIT_EXPORT Token Property_type(const Property *prop);

/// Sets type
PLUGKIT_EXPORT void Property_setType(Property *prop, Token type);

/// Gets value as a const pointer
PLUGKIT_EXPORT const Variant *Property_value(const Property *prop);

/// Gets value as a mutable pointer
PLUGKIT_EXPORT Variant *Property_valueRef(Property *prop);

/// Gets a boolean value
PLUGKIT_EXPORT bool Property_bool(const Property *prop);

/// Sets a boolean value
PLUGKIT_EXPORT void Property_setBool(Property *prop, bool value);

/// Gets an integer value
PLUGKIT_EXPORT int32_t Property_int32(const Property *prop);

/// Sets an integer value
PLUGKIT_EXPORT void Property_setInt32(Property *prop, int32_t value);

/// Gets an integer value
PLUGKIT_EXPORT int64_t Property_int64(const Property *prop);

/// Sets an integer value
PLUGKIT_EXPORT void Property_setInt64(Property *prop, int64_t value);

/// Gets an unsigned integer value
PLUGKIT_EXPORT uint32_t Property_uint32(const Property *prop);

/// Gets an unsigned integer value
PLUGKIT_EXPORT void Property_setUint32(Property *prop, uint32_t value);

/// Gets an unsigned integer value
PLUGKIT_EXPORT uint64_t Property_uint64(const Property *prop);

/// Gets an unsigned integer value
PLUGKIT_EXPORT void Property_setUint64(Property *prop, uint64_t value);

/// Gets a floating number value
PLUGKIT_EXPORT double Property_double(const Property *prop);

/// Sets a floating number value
PLUGKIT_EXPORT void Property_setDouble(Property *prop, double value);

/// Gets a string value
PLUGKIT_EXPORT const char *Property_string(const Property *prop);

/// Sets a string value
PLUGKIT_EXPORT void Property_setString(Property *prop, const char *str);

/// Sets a slice value
PLUGKIT_EXPORT Slice Property_slice(const Property *prop);

/// Sets a slice value
PLUGKIT_EXPORT void Property_setSlice(Property *prop, Slice slice);

PLUGKIT_EXPORT const Variant *Property_arrayValue(const Property *prop,
                                                  size_t index);

/// Gets a value associated with a given key
PLUGKIT_EXPORT const Variant *Property_mapValue(const Property *prop,
                                                const char *key, int length);

/// Gets a mutable value associated with a given key
PLUGKIT_EXPORT Variant *Property_mapValueRef(Property *prop, const char *key,
                                             int length);

PLUGKIT_NAMESPACE_END

#endif
