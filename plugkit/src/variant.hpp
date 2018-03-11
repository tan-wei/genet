#ifndef PLUGKIT_VARIANT_HPP
#define PLUGKIT_VARIANT_HPP

#include "slice.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <v8.h>
#include <vector>

namespace plugkit {

enum VariantType {
  VARTYPE_NIL = 0,
  VARTYPE_BOOL = 1,
  VARTYPE_INT64 = 2,
  VARTYPE_UINT64 = 3,
  VARTYPE_DOUBLE = 4,
  VARTYPE_STRING = 5,
  VARTYPE_STRING_REF = 6,
  VARTYPE_SLICE = 7,
  VARTYPE_ADDRESS = 12
};

struct Variant final {
public:
  Variant();
  Variant(bool value);
  Variant(int8_t value);
  Variant(uint8_t value);
  Variant(int16_t value);
  Variant(uint16_t value);
  Variant(int32_t value);
  Variant(uint32_t value);
  Variant(int64_t value);
  Variant(uint64_t value);
  Variant(double value);
  Variant(const Slice &slice);

  static Variant fromString(const char *str, size_t length);
  static Variant fromStringRef(const char *str, size_t length);
  static Variant fromAddress(void *ptr);
  Variant(void *) = delete;
  ~Variant();
  Variant(const Variant &value);
  Variant &operator=(const Variant &value);

public:
  VariantType type() const;
  bool isNil() const;
  bool isBool() const;
  bool isInt64() const;
  bool isUint64() const;
  bool isDouble() const;
  bool isString() const;
  bool isStringRef() const;
  bool isSlice() const;
  bool isAddress() const;

public:
  bool boolValue(bool defaultValue = bool()) const;
  int64_t int64Value(int64_t defaultValue = int64_t()) const;
  uint64_t uint64Value(uint64_t defaultValue = uint64_t()) const;
  double doubleValue(double defaultValue = double()) const;
  std::string string(const std::string &defaultValue = std::string()) const;
  Slice slice() const;
  void *address() const;
  uint64_t tag() const;
  size_t length() const;

public:
  static v8::Local<v8::Object> getNodeBuffer(const Slice &slice);
  static Slice getSlice(v8::Local<v8::ArrayBufferView> obj);
  static v8::Local<v8::Value> getValue(const Variant &var);
  static Variant getVariant(v8::Local<v8::Value> var);
  static void init(v8::Isolate *isolate);

public:
  uint64_t type_;
  union {
    bool bool_;
    double double_;
    int64_t int_;
    uint64_t uint_;
    std::shared_ptr<std::string> *str;
    const char *data;
    void *ptr;
  } d;
};

extern "C" {

/// Set the value of the variant to `Nil`.
void Variant_setNil(Variant *var);

/// Set the value of the variant to the given `bool` value.
void Variant_setBool(Variant *var, bool value);

/// Set the value of the variant to the given `int64_t` value.
void Variant_setInt64(Variant *var, int64_t value);

/// Set the value of the variant to the given `uint64_t` value.
void Variant_setUint64(Variant *var, uint64_t value);

/// Set the value of the variant to the given `double` value.
void Variant_setDouble(Variant *var, double value);

/// Return the value of the variant as a null-terminated string.
const char *Variant_string(const Variant *var, size_t *len);

/// Set the value of the variant to the given string.
///
/// If `length` is less than `0`,
/// the length of the string is determined by `strlen()`.
///
/// !> This function cannot handle a string contains NULL
/// even if a positive`length` is given,
/// because the given string will be copied as a null-terminated string.
void Variant_setString(Variant *var, const char *str, int length);

void Variant_setStringRef(Variant *var, const char *str, int length);

/// Set the value of the variant to the given `Slice` value.
///
/// !> Unlike `Variant_setString`,
/// this function does not make a copy of the buffer.
/// Be careful of its ownership.
void Variant_setSlice(Variant *var, Slice slice);
}

} // namespace plugkit

#endif
