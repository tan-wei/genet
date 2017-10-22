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
  VARTYPE_DOUBLE = 6,
  VARTYPE_STRING = 7,
  VARTYPE_SLICE = 8,
  VARTYPE_ARRAY = 9,
  VARTYPE_MAP = 10
} VariantType;

typedef struct Variant Variant;

/// Return the type of the variant.
PLUGKIT_EXPORT VariantType Variant_type(const Variant *var);

/// Set the value of the variant to `Nil`.
PLUGKIT_EXPORT void Variant_setNil(Variant *var);

/// Return the value of the variant as `bool`.
PLUGKIT_EXPORT bool Variant_bool(const Variant *var);

/// Set the value of the variant to the given `bool` value.
PLUGKIT_EXPORT void Variant_setBool(Variant *var, bool value);

/// Return the value of the variant as `int32_t`.
PLUGKIT_EXPORT int32_t Variant_int32(const Variant *var);

/// Set the value of the variant to the given `int32_t` value.
PLUGKIT_EXPORT void Variant_setInt32(Variant *var, int32_t value);

/// Return the value of the variant as `uint32_t`.
PLUGKIT_EXPORT uint32_t Variant_uint32(const Variant *var);

/// Set the value of the variant to the given `uint32_t` value.
PLUGKIT_EXPORT void Variant_setUint32(Variant *var, uint32_t value);

/// Return the value of the variant as `double`.
PLUGKIT_EXPORT double Variant_double(const Variant *var);

/// Set the value of the variant to the given `double` value.
PLUGKIT_EXPORT void Variant_setDouble(Variant *var, double value);

/// Return the value of the variant as a null-terminated string.
PLUGKIT_EXPORT const char *Variant_string(const Variant *var);

/// Set the value of the variant to the given string.
///
/// If `length` is less than `0`,
/// the length of the string is determined by `strlen()`.
///
/// !> This function cannot handle a string contains NULL
/// even if a positive`length` is given,
/// because the given string will be copied as a null-terminated string.
PLUGKIT_EXPORT void
Variant_setString(Variant *var, const char *str, int length);

/// Return the value of the variant as `Slice`.
PLUGKIT_EXPORT Slice Variant_slice(const Variant *var);

/// Set the value of the variant to the given `Slice` value.
///
/// !> Unlike `Variant_setString`,
/// this function does not make a copy of the buffer.
/// Be careful of its ownership.
PLUGKIT_EXPORT void Variant_setSlice(Variant *var, Slice slice);

/// Returns an element of the array at `index`.
/// If the variant is not an array or `index` is out of bounds, return `NULL`.
PLUGKIT_EXPORT const Variant *Variant_arrayValue(const Variant *var,
                                                 size_t index);

/// Returns a mutable element of the array at `index`.
///
/// The length of the array will be extended automatically.
///
/// If the variant is not an array,
/// the type of the variant become `array` even if another value is set.
PLUGKIT_EXPORT Variant *Variant_arrayValueRef(Variant *var, size_t index);

/// Returns an element of the map corresponded to `key`.
///
/// If `length` is less than `0`,
/// the length of the `key` is determined by `strlen()`.
///
/// If the variant is not a map or `key` is not found, return `NULL`.
PLUGKIT_EXPORT const Variant *
Variant_mapValue(const Variant *var, const char *key, int length);

/// Returns a mutable element of the map corresponded to `key`.
///
/// If `length` is less than `0`,
/// the length of the `key` is determined by `strlen()`.
///
/// If the variant is not a map,
/// the type of the variant become `map` even if another value is set.
PLUGKIT_EXPORT Variant *
Variant_mapValueRef(Variant *var, const char *key, int length);

PLUGKIT_NAMESPACE_END

#endif
