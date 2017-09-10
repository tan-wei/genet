#include <catch.hpp>
#include <cfloat>
#include <cstring>
#include "variant.hpp"

using namespace plugkit;

namespace {

TEST_CASE("Variant_bool", "[variant]") {
  Variant variant;
  CHECK(Variant_bool(&variant) == false);
  Variant_setBool(&variant, true);
  CHECK(Variant_bool(&variant) == true);
  Variant_setBool(&variant, false);
  CHECK(Variant_bool(&variant) == false);
  CHECK(Variant_bool(nullptr) == false);
}

TEST_CASE("Variant_int32", "[variant]") {
  Variant variant;
  CHECK(Variant_int32(&variant) == 0l);
  Variant_setInt32(&variant, INT32_MAX);
  CHECK(Variant_int32(&variant) == INT32_MAX);
  Variant_setInt32(&variant, UINT32_MAX);
  CHECK(Variant_int32(&variant) == -1);
  CHECK(Variant_int32(nullptr) == 0l);
}

TEST_CASE("Variant_uint32", "[variant]") {
  Variant variant;
  CHECK(Variant_uint32(&variant) == 0ul);
  Variant_setUint32(&variant, UINT32_MAX);
  CHECK(Variant_uint32(&variant) == UINT32_MAX);
  Variant_setUint32(&variant, INT32_MAX);
  CHECK(Variant_uint32(&variant) == INT32_MAX);
  Variant_setUint32(&variant, -1);
  CHECK(Variant_uint32(&variant) == UINT32_MAX);
  CHECK(Variant_int32(nullptr) == 0ul);
}

TEST_CASE("Variant_int64", "[variant]") {
  Variant variant;
  CHECK(Variant_int64(&variant) == 0ll);
  Variant_setInt64(&variant, INT64_MAX);
  CHECK(Variant_int64(&variant) == INT64_MAX);
  Variant_setInt64(&variant, UINT64_MAX);
  CHECK(Variant_int64(&variant) == -1);
  CHECK(Variant_int64(nullptr) == 0ll);
}

TEST_CASE("Variant_uint64", "[variant]") {
  Variant variant;
  CHECK(Variant_uint64(&variant) == 0ull);
  Variant_setUint64(&variant, UINT64_MAX);
  CHECK(Variant_uint64(&variant) == UINT64_MAX);
  Variant_setUint64(&variant, INT64_MAX);
  CHECK(Variant_uint64(&variant) == INT64_MAX);
  Variant_setUint64(&variant, -1);
  CHECK(Variant_uint64(&variant) == UINT64_MAX);
  CHECK(Variant_int64(nullptr) == 0ull);
}

TEST_CASE("Variant_double", "[variant]") {
  Variant variant;
  CHECK(Variant_double(&variant) == 0.0);
  Variant_setDouble(&variant, DBL_MAX);
  CHECK(Variant_double(&variant) == DBL_MAX);
  Variant_setDouble(&variant, DBL_MIN);
  CHECK(Variant_double(&variant) == DBL_MIN);
  Variant_setDouble(&variant, DBL_EPSILON);
  CHECK(Variant_double(&variant) == DBL_EPSILON);
  CHECK(Variant_int64(nullptr) == 0.0);
}

TEST_CASE("Variant_string", "[variant]") {
  Variant variant;
  CHECK(strcmp(Variant_string(&variant), "") == 0);
  Variant_setString(&variant, "HELLO", -1);
  CHECK(strcmp(Variant_string(&variant), "HELLO") == 0);
  CHECK(strcmp(Variant_string(nullptr), "") == 0);
  Variant_setString(&variant, "", -1);
  CHECK(strcmp(Variant_string(&variant), "") == 0);
}

TEST_CASE("Variant_slice", "[variant]") {
  Variant variant;
  char data[256];
  Slice slice = {data, data + sizeof(data)};
  Slice slice2 = Variant_slice(&variant);
  CHECK(slice2.begin == nullptr);
  CHECK(slice2.end == nullptr);
  Variant_setSlice(&variant, slice);
  slice2 = Variant_slice(&variant);
  CHECK(slice2.begin == slice.begin);
  CHECK(slice2.end == slice.end);
  slice2 = Variant_slice(nullptr);
  CHECK(slice2.begin == nullptr);
  CHECK(slice2.end == nullptr);
}

TEST_CASE("Variant_arrayValue", "[variant]") {
  Variant variant;
  const Variant *value = Variant_arrayValue(&variant, 0);
  CHECK(Variant_bool(value) == false);
  value = Variant_arrayValue(&variant, 1);
  CHECK(Variant_bool(value) == false);

  Variant *valueRef = Variant_arrayValueRef(&variant, 2);
  Variant_setBool(valueRef, true);
  value = Variant_arrayValue(&variant, 0);
  CHECK(Variant_bool(value) == false);
  value = Variant_arrayValue(&variant, 1);
  CHECK(Variant_bool(value) == false);
  value = Variant_arrayValue(&variant, 2);
  CHECK(Variant_bool(value) == true);
  value = Variant_arrayValue(&variant, 3);
  CHECK(Variant_bool(value) == false);
}

TEST_CASE("Variant_mapValue", "[variant]") {
  Variant variant;
  const Variant *value = Variant_mapValue(&variant, "aaa", -1);
  CHECK(Variant_bool(value) == false);
  value = Variant_mapValue(&variant, "bbb", -1);
  CHECK(Variant_bool(value) == false);

  Variant *valueRef = Variant_mapValueRef(&variant, "ccc", -1);
  Variant_setBool(valueRef, true);
  value = Variant_mapValue(&variant, "aaa", 3);
  CHECK(Variant_bool(value) == false);
  value = Variant_mapValue(&variant, "bbb", 3);
  CHECK(Variant_bool(value) == false);
  value = Variant_mapValue(&variant, "ccc", 3);
  CHECK(Variant_bool(value) == true);
  value = Variant_mapValue(&variant, "ddd", 3);
  CHECK(Variant_bool(value) == false);
}
}
