#include <catch.hpp>
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
}

TEST_CASE("Variant_int64", "[variant]") {
  Variant variant;
  CHECK(Variant_int64(&variant) == 0ll);
  Variant_setInt64(&variant, INT64_MAX);
  CHECK(Variant_int64(&variant) == INT64_MAX);
  Variant_setInt64(&variant, UINT64_MAX);
  CHECK(Variant_int64(&variant) == -1);
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
}
}
