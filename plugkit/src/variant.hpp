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
  VARTYPE_SLICE = 6,
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
  static Variant fromAddress(void *ptr);
  Variant(void *) = delete;
  ~Variant() = default;
  Variant(const Variant &value) = default;
  Variant &operator=(const Variant &value) = default;

public:
  VariantType type() const;
  bool isNil() const;
  bool isBool() const;
  bool isInt64() const;
  bool isUint64() const;
  bool isDouble() const;
  bool isString() const;
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

public:
  uint64_t type_;
  union {
    bool bool_;
    double double_;
    int64_t int_;
    uint64_t uint_;
    const char *data;
    void *ptr;
  } d;
};
} // namespace plugkit

#endif
