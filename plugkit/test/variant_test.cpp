#include "variant.hpp"
#include <catch.hpp>
#include <cfloat>
#include <cstring>

using namespace plugkit;

namespace {

TEST_CASE("Variant_type", "[variant]") {
  Variant variant;
  CHECK(Variant_type(&variant) == VARTYPE_NIL);
  Variant_setBool(&variant, false);
  CHECK(Variant_type(&variant) == VARTYPE_BOOL);
  Variant_setInt64(&variant, 0);
  CHECK(Variant_type(&variant) == VARTYPE_INT64);
  Variant_setUint64(&variant, 0);
  CHECK(Variant_type(&variant) == VARTYPE_UINT64);
  Variant_setDouble(&variant, 0.0);
  CHECK(Variant_type(&variant) == VARTYPE_DOUBLE);
  Variant_setString(&variant, "HELLO", 5);
  CHECK(Variant_type(&variant) == VARTYPE_STRING);
  Variant_setStringRef(&variant, "HELLO", 5);
  CHECK(Variant_type(&variant) == VARTYPE_STRING_REF);
  Variant_setSlice(&variant, Slice());
  CHECK(Variant_type(&variant) == VARTYPE_SLICE);
  Variant_setNil(&variant);
  CHECK(Variant_type(&variant) == VARTYPE_NIL);
}

TEST_CASE("Variant_setNil", "[variant]") {
  Variant variant;
  Variant_setNil(&variant);
  CHECK(Variant_type(&variant) == VARTYPE_NIL);
}

TEST_CASE("Variant_bool", "[variant]") {
  Variant variant;
  CHECK(Variant_bool(&variant) == false);
  Variant_setBool(&variant, true);
  CHECK(Variant_bool(&variant) == true);
  Variant_setBool(&variant, false);
  CHECK(Variant_bool(&variant) == false);
  CHECK(Variant_bool(nullptr) == false);
}

TEST_CASE("Variant_int64", "[variant]") {
  Variant variant;
  CHECK(Variant_int64(&variant) == 0l);
  Variant_setInt64(&variant, INT64_MAX);
  CHECK(Variant_int64(&variant) == INT64_MAX);
  Variant_setInt64(&variant, UINT64_MAX);
  CHECK(Variant_int64(&variant) == -1);
  CHECK(Variant_int64(nullptr) == 0l);
}

TEST_CASE("Variant_uint64", "[variant]") {
  Variant variant;
  CHECK(Variant_uint64(&variant) == 0ul);
  Variant_setUint64(&variant, UINT64_MAX);
  CHECK(Variant_uint64(&variant) == UINT64_MAX);
  Variant_setUint64(&variant, INT64_MAX);
  CHECK(Variant_uint64(&variant) == INT64_MAX);
  Variant_setUint64(&variant, -1);
  CHECK(Variant_uint64(&variant) == UINT64_MAX);
  CHECK(Variant_int64(nullptr) == 0ul);
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
  CHECK(Variant_double(nullptr) == 0.0);
}

TEST_CASE("Variant_string", "[variant]") {
  Variant variant;
  size_t len = 0;
  CHECK(strcmp(Variant_string(&variant, &len), "") == 0);
  CHECK(len == 0);
  Variant_setString(&variant, "HELLO", 5);
  CHECK(strcmp(Variant_string(&variant, &len), "HELLO") == 0);
  CHECK(len == 5);
  Variant_setString(&variant, "HELLO___", 8);
  CHECK(strcmp(Variant_string(&variant, &len), "HELLO___") == 0);
  CHECK(len == 8);
  Variant_setString(&variant, "HELLO WORLD", 11);
  CHECK(strcmp(Variant_string(&variant, &len), "HELLO WORLD") == 0);
  CHECK(len == 11);
  CHECK(strcmp(Variant_string(nullptr, &len), "") == 0);
  CHECK(len == 0);
  Variant_setString(&variant, "", 0);
  CHECK(strcmp(Variant_string(&variant, &len), "") == 0);
  CHECK(len == 0);
}

TEST_CASE("Variant_setStringRef", "[variant]") {
  Variant variant;
  size_t len = 0;
  CHECK(strcmp(Variant_string(&variant, &len), "") == 0);
  CHECK(len == 0);
  Variant_setStringRef(&variant, "HELLO", 3);
  CHECK(strcmp(Variant_string(&variant, &len), "HELLO") == 0);
  CHECK(len == 3);
  Variant_setStringRef(&variant, "HELLO___", 2);
  CHECK(strcmp(Variant_string(&variant, &len), "HELLO___") == 0);
  CHECK(len == 2);
  Variant_setStringRef(&variant, "HELLO WORLD", 10);
  CHECK(strcmp(Variant_string(&variant, &len), "HELLO WORLD") == 0);
  CHECK(len == 10);
  CHECK(strcmp(Variant_string(nullptr, &len), "") == 0);
  CHECK(len == 0);
  Variant_setStringRef(&variant, "", 0);
  CHECK(strcmp(Variant_string(&variant, &len), "") == 0);
  CHECK(len == 0);
}

TEST_CASE("Variant_slice", "[variant]") {
  Variant variant;
  char data[256];
  Slice slice = {data, sizeof(data)};
  Slice slice2 = Variant_slice(&variant);
  CHECK(slice2.data == nullptr);
  CHECK(slice2.length == 0);
  Variant_setSlice(&variant, slice);
  slice2 = Variant_slice(&variant);
  CHECK(slice2.data == slice.data);
  CHECK(slice2.length == slice.length);
  slice2 = Variant_slice(nullptr);
  CHECK(slice2.data == nullptr);
  CHECK(slice2.length == 0);
}
} // namespace
