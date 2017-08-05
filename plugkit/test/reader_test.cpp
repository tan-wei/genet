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

TEST_CASE("Reader_sliceAll", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[256];
  View view = {data, data + sizeof(data)};
  reader.view = view;

  View subview = Reader_sliceAll(&reader, 12);
  CHECK(subview.begin == data + 12);
  CHECK(subview.end == data + sizeof(data));
  CHECK(reader.lastRange.begin == 12);
  CHECK(reader.lastRange.end == sizeof(data));
  CHECK(reader.lastError.type == Token_null());

  subview = Reader_sliceAll(&reader, 12);
  CHECK(subview.begin == nullptr);
  CHECK(subview.end == nullptr);
  CHECK(reader.lastRange.begin == 12);
  CHECK(reader.lastRange.end == sizeof(data));
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

  CHECK(Reader_readUint8(&reader) == uint8_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint8_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readInt8", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-100};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readInt8(&reader) == -100);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int8_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt8(&reader) == int8_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int8_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readUint16BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 5};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readUint16BE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint16BE(&reader) == uint16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readUint32BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 5};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readUint32BE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint32BE(&reader) == uint32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readUint64BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 0, 0, 0, 0, 5};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readUint64BE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint64BE(&reader) == uint64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readInt16BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readInt16BE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt16BE(&reader) == int16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readInt32BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readInt32BE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt32BE(&reader) == int32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readInt64BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readInt64BE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt64BE(&reader) == int64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readFloat32BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-64, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readFloat32BE(&reader) == -2.0f);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat32BE(&reader) == float());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readFloat64BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-64, 0, 0, 0, 0, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readFloat64BE(&reader) == -2.0);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat64BE(&reader) == double());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readUint16LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readUint16LE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint16LE(&reader) == uint16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readUint32LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readUint32LE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint32LE(&reader) == uint32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readUint64LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0, 0, 0, 0, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readUint64LE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint64LE(&reader) == uint64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readInt16LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, static_cast<char>(255)};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readInt16LE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt16LE(&reader) == int16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readInt32LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255)};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readInt32LE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt32LE(&reader) == int32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readInt64LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0,
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255)};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readInt64LE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt64LE(&reader) == int64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readFloat32LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, -64};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readFloat32LE(&reader) == -2.0f);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat32LE(&reader) == float());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == outOfBoundError());
}

TEST_CASE("Reader_readFloat64LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 0, 0, 0, 0, -64};
  View view = {data, data + sizeof(data)};
  reader.view = view;

  CHECK(Reader_readFloat64LE(&reader) == -2.0);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat64LE(&reader) == double());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == outOfBoundError());
}
}
