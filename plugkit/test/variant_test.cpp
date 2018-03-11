#include "variant.hpp"
#include <catch.hpp>
#include <cfloat>
#include <cstring>

using namespace plugkit;

namespace {
TEST_CASE("Variant_setNil", "[variant]") {
  Variant variant;
  Variant_setNil(&variant);
  CHECK(variant.type() == VARTYPE_NIL);
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

} // namespace
