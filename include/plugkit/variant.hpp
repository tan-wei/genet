#ifndef PLUGKIT_VARIANT_H
#define PLUGKIT_VARIANT_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include "slice.hpp"
#include "stream_buffer.hpp"

namespace plugkit {

class Variant final {
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
  using Map = std::unordered_map<std::string, Variant>;
  using Timestamp = std::chrono::time_point<std::chrono::system_clock,
                                            std::chrono::nanoseconds>;

public:
  Variant();
  Variant(bool value);
  Variant(int64_t value);
  Variant(uint64_t value);
  Variant(double value);
  Variant(const std::string &str);
  Variant(std::string &&str);
  Variant(const Slice &slice);
  Variant(const StreamBuffer &stream);
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
  bool boolValue() const;
  int64_t int64Value() const;
  uint64_t uint64Value() const;
  double doubleValue() const;
  Timestamp timestamp() const;
  std::string string() const;
  Slice slice() const;
  StreamBuffer stream() const;
  const Array &array() const;
  const Map &map() const;
  uint8_t tag() const;
  Variant operator[](size_t index) const;
  Variant operator[](const std::string &key) const;
  size_t length() const;

public:
  class Private;

private:
  uint8_t type_;
  int8_t tag_;
  union {
    bool bool_;
    double double_;
    int64_t int_;
    uint64_t uint_;
    Timestamp *ts;
    Slice *slice;
    Array *array;
    Map *map;
  } d;
};
}

#endif
