#ifndef PLUGKIT_VARIANT_H
#define PLUGKIT_VARIANT_H

#include <cstdint>
#include <cstddef>

extern "C" {

namespace plugkit {

class Variant;

bool Variant_bool(const Variant *var);
void Variant_setBool(Variant *var, bool value);

int64_t Variant_int64(const Variant *var);
void Variant_setInt64(Variant *var, int64_t value);

uint64_t Variant_uint64(const Variant *var);
void Variant_setUint64(Variant *var, uint64_t value);

double Variant_double(const Variant *var);
void Variant_setDouble(Variant *var, double value);

const char *Variant_string(const Variant *var);
void Variant_setString(Variant *var, const char *str);

size_t Variant_data(const Variant *var, const char **data);
void Variant_setDataCopy(Variant *var, const char *data, size_t length);
}
}

#endif
