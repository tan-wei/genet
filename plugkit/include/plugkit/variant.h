#ifndef PLUGKIT_VARIANT_H
#define PLUGKIT_VARIANT_H

#include "slice.h"
#include <cstdint>

extern "C" {

namespace plugkit {

class Variant;

/// Gets a boolean value
bool Variant_bool(const Variant *var);

/// Sets a boolean value
void Variant_setBool(Variant *var, bool value);

/// Gets an integer value
int64_t Variant_int64(const Variant *var);

/// Sets an integer value
void Variant_setInt64(Variant *var, int64_t value);

/// Gets an unsigned integer value
uint64_t Variant_uint64(const Variant *var);

/// Gets an unsigned integer value
void Variant_setUint64(Variant *var, uint64_t value);

/// Gets a floating number value
double Variant_double(const Variant *var);

/// Sets a floating number value
void Variant_setDouble(Variant *var, double value);

/// Gets a string value
const char *Variant_string(const Variant *var);

/// Sets a string value
void Variant_setString(Variant *var, const char *str);

Slice Variant_data(const Variant *var);
void Variant_setData(Variant *var, Slice slice);
}
}

#endif
