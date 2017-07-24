#include <catch.hpp>
#include "listener_status.hpp"
#include "attribute.hpp"

using namespace plugkit;

namespace {

TEST_CASE("ListenerStatus", "revision") {
  ListenerStatus status;
  CHECK(status.revision() == 0);

  int count = 256;
  for (int i = 0; i < count; ++i) {
    status.addAttribute(std::make_shared<Attribute>(MID("test")));
  }
  CHECK(status.revision() == count);
}
}
