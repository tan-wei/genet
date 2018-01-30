#ifndef PLUGKIT_VARIANT_HPP
#define PLUGKIT_VARIANT_HPP

#include "slice.hpp"
#include <json11.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <v8.h>
#include <vector>

namespace plugkit {

enum VariantType {
  VARTYPE_NIL = 0,
  VARTYPE_BOOL = 1,
  VARTYPE_INT32 = 2,
  VARTYPE_UINT32 = 3,
  VARTYPE_DOUBLE = 6,
  VARTYPE_STRING = 7,
  VARTYPE_SLICE = 8,
  VARTYPE_ARRAY = 9,
  VARTYPE_MAP = 10
};

struct Variant final {
public:
  using Array = std::vector<Variant>;
  using Map = std::unordered_map<std::string, Variant>;

public:
  Variant();
  Variant(bool value);
  Variant(int8_t value);
  Variant(uint8_t value);
  Variant(int16_t value);
  Variant(uint16_t value);
  Variant(int32_t value);
  Variant(uint32_t value);
  Variant(double value);
  Variant(const std::string &str);
  Variant(const Slice &slice);
  Variant(const Array &array);
  Variant(const Map &map);
  Variant(void *) = delete;
  ~Variant();
  Variant(const Variant &value);
  Variant &operator=(const Variant &value);

public:
  VariantType type() const;
  bool isNil() const;
  bool isBool() const;
  bool isInt32() const;
  bool isUint32() const;
  bool isDouble() const;
  bool isString() const;
  bool isSlice() const;
  bool isArray() const;
  bool isMap() const;

public:
  bool boolValue(bool defaultValue = bool()) const;
  int32_t int32Value(int32_t defaultValue = int32_t()) const;
  uint32_t uint32Value(uint32_t defaultValue = uint32_t()) const;
  double doubleValue(double defaultValue = double()) const;
  std::string string(const std::string &defaultValue = std::string()) const;
  Slice slice() const;
  const Array &array() const;
  const Map &map() const;
  uint8_t tag() const;
  Variant operator[](size_t index) const;
  Variant &operator[](size_t index);
  Variant operator[](const std::string &key) const;
  Variant &operator[](const std::string &key);
  size_t length() const;

public:
  static v8::Local<v8::Object> getNodeBuffer(const Slice &slice);
  static Slice getSlice(v8::Local<v8::ArrayBufferView> obj);
  static v8::Local<v8::Value> getValue(const Variant &var);
  static Variant getVariant(v8::Local<v8::Value> var);
  static json11::Json getJson(const Variant &var);
  static void init(v8::Isolate *isolate);

public:
  uint8_t type_;
  union {
    bool bool_;
    double double_;
    int32_t int_;
    uint32_t uint_;
    std::shared_ptr<std::string> *str;
    Slice *slice;
    Array *array;
    Map *map;
  } d;
};

extern "C" {

/// Return the type of the variant.
VariantType Variant_type(const Variant *var);

/// Set the value of the variant to `Nil`.
void Variant_setNil(Variant *var);

/// Return the value of the variant as `bool`.
bool Variant_bool(const Variant *var);

/// Set the value of the variant to the given `bool` value.
void Variant_setBool(Variant *var, bool value);

/// Return the value of the variant as `int32_t`.
int32_t Variant_int32(const Variant *var);

/// Set the value of the variant to the given `int32_t` value.
void Variant_setInt32(Variant *var, int32_t value);

/// Return the value of the variant as `uint32_t`.
uint32_t Variant_uint32(const Variant *var);

/// Set the value of the variant to the given `uint32_t` value.
void Variant_setUint32(Variant *var, uint32_t value);

/// Return the value of the variant as `double`.
double Variant_double(const Variant *var);

/// Set the value of the variant to the given `double` value.
void Variant_setDouble(Variant *var, double value);

/// Return the value of the variant as a null-terminated string.
const char *Variant_string(const Variant *var);

/// Set the value of the variant to the given string.
///
/// If `length` is less than `0`,
/// the length of the string is determined by `strlen()`.
///
/// !> This function cannot handle a string contains NULL
/// even if a positive`length` is given,
/// because the given string will be copied as a null-terminated string.
void Variant_setString(Variant *var, const char *str, int length);

/// Return the value of the variant as `Slice`.
Slice Variant_slice(const Variant *var);

/// Set the value of the variant to the given `Slice` value.
///
/// !> Unlike `Variant_setString`,
/// this function does not make a copy of the buffer.
/// Be careful of its ownership.
void Variant_setSlice(Variant *var, Slice slice);

/// Returns an element of the array at `index`.
/// If the variant is not an array or `index` is out of bounds, return `NULL`.
const Variant *Variant_arrayValue(const Variant *var, size_t index);

/// Returns a mutable element of the array at `index`.
///
/// The length of the array will be extended automatically.
///
/// If the variant is not an array,
/// the type of the variant become `array` even if another value is set.
Variant *Variant_arrayValueRef(Variant *var, size_t index);

/// Returns an element of the map corresponded to `key`.
///
/// If `length` is less than `0`,
/// the length of the `key` is determined by `strlen()`.
///
/// If the variant is not a map or `key` is not found, return `NULL`.
const Variant *
Variant_mapValue(const Variant *var, const char *key, int length);

/// Returns a mutable element of the map corresponded to `key`.
///
/// If `length` is less than `0`,
/// the length of the `key` is determined by `strlen()`.
///
/// If the variant is not a map,
/// the type of the variant become `map` even if another value is set.
Variant *Variant_mapValueRef(Variant *var, const char *key, int length);
}

} // namespace plugkit

#endif
