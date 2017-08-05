#include <catch.hpp>
#include "reader.h"

using namespace plugkit;

namespace {

Token outOfBoundError() {
  static const Token token = Token_get("Out of bound");
  return token;
}

TEST_CASE("Reader_reset", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  CHECK(reader.view.begin == nullptr);
  CHECK(reader.view.end == nullptr);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == 0);
  CHECK(reader.lastError.type == Token_null());
}

TEST_CASE("Reader_slice", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[256];
  View view = {data, data + sizeof(data)};
  reader.view = view;

  View subview = Reader_slice(&reader, 1, 54);
  CHECK(subview.begin == data + 1);
  CHECK(subview.end == data + 1 + 54);
  CHECK(reader.lastRange.begin == 1);
  CHECK(reader.lastRange.end == 1 + 54);
  CHECK(reader.lastError.type == Token_null());

  subview = Reader_slice(&reader, 10, 64);
  CHECK(subview.begin == data + 1 + 54 + 10);
  CHECK(subview.end == data + 1 + 54 + 10 + 64);
  CHECK(reader.lastRange.begin == 1 + 54 + 10);
  CHECK(reader.lastRange.end == 1 + 54 + 10 + 64);
  CHECK(reader.lastError.type == Token_null());

  subview = Reader_slice(&reader, 100, 64);
  CHECK(subview.begin == nullptr);
  CHECK(subview.end == nullptr);
  CHECK(reader.lastRange.begin == 1 + 54 + 10);
  CHECK(reader.lastRange.end == 1 + 54 + 10 + 64);
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readUint8", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(128)};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readUint8(&reader) == 128);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint8_t));
  CHECK(reader.lastError.type == Token_null());

  view.begin = data;
  view.end = data;
  CHECK(Reader_readUint8(&reader) == uint8_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint8_t));
  CHECK(reader.lastError.type == outOfBoundError());
}
}
