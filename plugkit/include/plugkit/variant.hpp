#ifndef PLUGKIT_VARIANT_HPP
#define PLUGKIT_VARIANT_HPP

#include "variant.h"
#include "slice.hpp"
#include "types.hpp"
#include "export.hpp"
#include <memory>
#include <string>
#include <vector>

namespace plugkit {

class PLUGKIT_EXPORT Variant final {
public:
  enum Type {
    TYPE_NIL,
    TYPE_BOOL,
    TYPE_INT64,
    TYPE_UINT64,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_TIMESTAMP,
    TYPE_BUFFER,
    TYPE_STREAM,
    TYPE_ARRAY,
    TYPE_MAP
  };
  using Array = std::vector<Variant>;
  using Map = std::vector<std::pair<std::string, Variant>>;

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
  Variant(const std::string &str);
  Variant(std::string &&str);
  Variant(const Slice &slice);
  Variant(const Array &array);
  Variant(const Map &map);
  Variant(Array &&array);
  Variant(Map &&map);
  Variant(const Timestamp &ts);
  Variant(void *) = delete;
  ~Variant();
  Variant(const Variant &value);
  Variant &operator=(const Variant &value);

public:
  Type type() const;
  bool isNil() const;
  bool isBool() const;
  bool isInt64() const;
  bool isUint64() const;
  bool isDouble() const;
  bool isString() const;
  bool isTimestamp() const;
  bool isBuffer() const;
  bool isStream() const;
  bool isArray() const;
  bool isMap() const;

public:
  bool boolValue(bool defaultValue = bool()) const;
  int64_t int64Value(int64_t defaultValue = int64_t()) const;
  uint64_t uint64Value(uint64_t defaultValue = uint64_t()) const;
  double doubleValue(double defaultValue = double()) const;
  Timestamp timestamp(const Timestamp &defaultValue = Timestamp()) const;
  std::string string(const std::string &defaultValue = std::string()) const;
  Slice slice() const;
  const Array &array() const;
  const Map &map() const;
  uint8_t tag() const;
  Variant operator[](size_t index) const;
  Variant operator[](const std::string &key) const;
  size_t length() const;

public:
  class Private;

public:
  uint8_t type_;
  int8_t tag_;
  union {
    bool bool_;
    double double_;
    int64_t int_;
    uint64_t uint_;
    Timestamp *ts;
    std::string *str;
    Slice *slice;
    Array *array;
    Map *map;
  } d;
};
}

#endif
