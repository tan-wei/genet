#ifndef PLUGKIT_VARIANT_H
#define PLUGKIT_VARIANT_H

#include <stdint.h>
#include <stdbool.h>
#include "slice.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

struct Variant;
typedef struct Variant Variant;

/// Gets a boolean value
PLUGKIT_EXPORT bool Variant_bool(const Variant *var);

/// Sets a boolean value
PLUGKIT_EXPORT void Variant_setBool(Variant *var, bool value);

/// Gets an integer value
PLUGKIT_EXPORT int64_t Variant_int64(const Variant *var);

/// Sets an integer value
PLUGKIT_EXPORT void Variant_setInt64(Variant *var, int64_t value);

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
PLUGKIT_EXPORT void Variant_setString(Variant *var, const char *str);

/// Gets a slice value
PLUGKIT_EXPORT Slice Variant_slice(const Variant *var);

/// Sets a slice value
PLUGKIT_EXPORT void Variant_setSlice(Variant *var, Slice slice);

/// Returns the first address of arrays
/// and assigns the length of arrays to size.
PLUGKIT_EXPORT const Variant *Variant_valueAt(const Variant *var, size_t index);

/// Gets a value associated with a given key
PLUGKIT_EXPORT const Variant *Variant_mapValue(const Variant *var,
                                               const char *key);

PLUGKIT_NAMESPACE_END

#endif
