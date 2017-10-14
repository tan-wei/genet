#ifndef PLUGKIT_VARIANT_H
#define PLUGKIT_VARIANT_H

#include "export.h"
#include "slice.h"
#include <stdbool.h>
#include <stdint.h>

PLUGKIT_NAMESPACE_BEGIN

typedef enum VariantType {
  VARTYPE_NIL = 0,
  VARTYPE_BOOL = 1,
  VARTYPE_INT32 = 2,
  VARTYPE_UINT32 = 3,
  VARTYPE_INT64 = 4,
  VARTYPE_UINT64 = 5,
  VARTYPE_DOUBLE = 6,
  VARTYPE_STRING = 7,
  VARTYPE_SLICE = 8,
  VARTYPE_ARRAY = 9,
  VARTYPE_MAP = 10
} VariantType;

typedef struct Variant Variant;

/// Variant type
PLUGKIT_EXPORT VariantType Variant_type(const Variant *var);

/// Sets a nil value
PLUGKIT_EXPORT void Variant_setNil(Variant *var);

/// Gets a boolean value
PLUGKIT_EXPORT bool Variant_bool(const Variant *var);

/// Sets a boolean value
PLUGKIT_EXPORT void Variant_setBool(Variant *var, bool value);

/// Gets an integer value
PLUGKIT_EXPORT int32_t Variant_int32(const Variant *var);

/// Sets an integer value
PLUGKIT_EXPORT void Variant_setInt32(Variant *var, int32_t value);

/// Gets an integer value
PLUGKIT_EXPORT int64_t Variant_int64(const Variant *var);

/// Sets an integer value
PLUGKIT_EXPORT void Variant_setInt64(Variant *var, int64_t value);

/// Gets an unsigned integer value
PLUGKIT_EXPORT uint32_t Variant_uint32(const Variant *var);

/// Gets an unsigned integer value
PLUGKIT_EXPORT void Variant_setUint32(Variant *var, uint32_t value);

/// Gets an unsigned integer value
PLUGKIT_EXPORT uint64_t Variant_uint64(const Variant *var);

/// Gets an unsigned integer value
PLUGKIT_EXPORT void Variant_setUint64(Variant *var, uint64_t value);

/// Gets a floating number value
PLUGKIT_EXPORT double Variant_double(const Variant *var);

/// Sets a floating number value
PLUGKIT_EXPORT void Variant_setDouble(Variant *var, double value);

/// Gets a string value
PLUGKIT_EXPORT const char *Variant_string(const Variant *var);

/// Sets a string value
PLUGKIT_EXPORT void
Variant_setString(Variant *var, const char *str, int length);

/// Gets a slice value
PLUGKIT_EXPORT Slice Variant_slice(const Variant *var);

/// Sets a slice value
PLUGKIT_EXPORT void Variant_setSlice(Variant *var, Slice slice);

/// Returns the first address of arrays
/// and assigns the length of arrays to size.
PLUGKIT_EXPORT const Variant *Variant_arrayValue(const Variant *var,
                                                 size_t index);

PLUGKIT_EXPORT Variant *Variant_arrayValueRef(Variant *var, size_t index);

/// Gets a value associated with a given key
PLUGKIT_EXPORT const Variant *
Variant_mapValue(const Variant *var, const char *key, int length);

/// Gets a mutable value associated with a given key
PLUGKIT_EXPORT Variant *
Variant_mapValueRef(Variant *var, const char *key, int length);

PLUGKIT_NAMESPACE_END

#endif
