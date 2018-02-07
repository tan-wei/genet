#include "tag_filter.hpp"
#include "token.hpp"
#include <catch.hpp>

using namespace plugkit;

namespace {

TEST_CASE("TagFilter_match", "[TagFilter]") {
  TagFilter filter(
      {Token_get("[eth]"), Token_get("[tcp]"), Token_get("[ipv4]")});
  CHECK(filter.match({Token_get("[eth]"), Token_get("[tcp]"),
                      Token_get("[ipv4]")}) == true);
  CHECK(filter.match({Token_get("[tcp]")}) == false);
  CHECK(filter.match({Token_get("[udp]"), Token_get("[eth]"),
                      Token_get("[tcp]"), Token_get("[ipv4]")}) == true);
  CHECK(filter.match({Token_get("[ipv4]")}) == false);
  CHECK(filter.match({Token_get("[ipv6]"), Token_get("[eth]"),
                      Token_get("[tcp]"), Token_get("[ipv4]")}) == true);
  CHECK(filter.match({Token_get("eth")}) == false);
  CHECK(filter.match({Token_get("tcp")}) == false);
  CHECK(filter.match({Token_get("udp")}) == false);
  CHECK(filter.match({Token_get("ipv4")}) == false);
  CHECK(filter.match({Token_get("ipv6")}) == false);
  CHECK(filter.match({Token_null()}) == false);
}
} // namespace
