#include "payload.h"
#include "payload.hpp"
#include <catch.hpp>

using namespace plugkit;

namespace {

TEST_CASE("Payload_type", "[Payload]") {
  Payload payload;

  Token token = Token_get("@stream");
  Payload_setType(&payload, token);
  CHECK(Payload_type(&payload) == token);
}

TEST_CASE("Payload_slices", "[Payload]") {
  Payload payload;
  char data[256];
  size_t size;
  const Slice *slice = Payload_slices(&payload, &size);
  CHECK(slice != nullptr);
  CHECK(size == 0);
  CHECK(slice[0].begin == nullptr);
  CHECK(slice[0].end == nullptr);
  Payload_addSlice(&payload, Slice{data, data + sizeof(data)});
  slice = Payload_slices(&payload, &size);
  CHECK(slice != nullptr);
  CHECK(size == 1);
  CHECK(slice[0].begin == data);
  CHECK(slice[0].end == data + sizeof(data));
  Payload_addSlice(&payload, Slice{data, data + 100});
  slice = Payload_slices(&payload, &size);
  CHECK(slice != nullptr);
  CHECK(size == 2);
  CHECK(slice[0].begin == data);
  CHECK(slice[0].end == data + sizeof(data));
  CHECK(slice[1].begin == data);
  CHECK(slice[1].end == data + 100);
}

TEST_CASE("Payload_range", "[Payload]") {
  Payload payload;

  Range range = Payload_range(&payload);
  CHECK(range.begin == 0);
  CHECK(range.end == 0);
  Payload_setRange(&payload, Range{123, 456});
  range = Payload_range(&payload);
  CHECK(range.begin == 123);
  CHECK(range.end == 456);
}

} // namespace
