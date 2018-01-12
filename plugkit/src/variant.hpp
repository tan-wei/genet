#ifndef PLUGKIT_VARIANT_HPP
#define PLUGKIT_VARIANT_HPP

#include "variant.h"
#include <json11.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <v8.h>
#include <vector>

namespace plugkit {

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
  uint8_t type_ : 4;
  uint8_t tag_ : 4;
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
} // namespace plugkit

#endif
