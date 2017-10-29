#include "reader.h"
#include <catch.hpp>

#ifdef WIN32
#pragma warning(disable : 4309)
#endif

using namespace plugkit;

namespace {

TEST_CASE("Reader_reset", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  CHECK(reader.data.begin == nullptr);
  CHECK(reader.data.end == nullptr);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == 0);
  CHECK(reader.lastError == Token_null());
}

TEST_CASE("Reader_slice", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[256];
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  Slice subview = Reader_slice(&reader, 1, 55);
  CHECK(subview.begin == data + 1);
  CHECK(subview.end == data + 1 + 54);
  CHECK(reader.lastRange.begin == 1);
  CHECK(reader.lastRange.end == 1 + 54);
  CHECK(reader.lastError == Token_null());

  subview = Reader_slice(&reader, 10, 74);
  CHECK(subview.begin == data + 1 + 54 + 10);
  CHECK(subview.end == data + 1 + 54 + 10 + 64);
  CHECK(reader.lastRange.begin == 1 + 54 + 10);
  CHECK(reader.lastRange.end == 1 + 54 + 10 + 64);
  CHECK(reader.lastError == Token_null());

  subview = Reader_slice(&reader, 100, 164);
  CHECK(subview.begin == nullptr);
  CHECK(subview.end == nullptr);
  CHECK(reader.lastRange.begin == 1 + 54 + 10);
  CHECK(reader.lastRange.end == 1 + 54 + 10 + 64);
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_sliceAll", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[256];
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  Slice subview = Reader_sliceAll(&reader, 12);
  CHECK(subview.begin == data + 12);
  CHECK(subview.end == data + sizeof(data));
  CHECK(reader.lastRange.begin == 12);
  CHECK(reader.lastRange.end == sizeof(data));
  CHECK(reader.lastError == Token_null());

  subview = Reader_sliceAll(&reader, 12);
  CHECK(subview.begin == nullptr);
  CHECK(subview.end == nullptr);
  CHECK(reader.lastRange.begin == 12);
  CHECK(reader.lastRange.end == sizeof(data));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getUint8", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(128)};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getUint8(&reader) == 128);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint8_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getUint8(&reader) == uint8_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint8_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getInt8", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-100};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getInt8(&reader) == -100);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int8_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getInt8(&reader) == int8_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int8_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getUint16", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 5};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getUint16(&reader, false) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getUint16(&reader, false) == uint16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getUint32", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getUint32(&reader, false) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getUint32(&reader, false) == uint32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getUint64", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 0, 0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getUint64(&reader, false) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getUint64(&reader, false) == uint64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getInt16", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getInt16(&reader, false) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getInt16(&reader, false) == int16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getInt32", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getInt32(&reader, false) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getInt32(&reader, false) == int32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getInt64", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getInt64(&reader, false) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getInt64(&reader, false) == int64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getFloat32", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-64, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getFloat32(&reader, false) == -2.0f);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getFloat32(&reader, false) == float());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getFloat64", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-64, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getFloat64(&reader, false) == -2.0);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getFloat64(&reader, false) == double());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getUint16 (little endian)", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getUint16(&reader, true) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getUint16(&reader, true) == uint16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getUint32 (little endian)", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getUint32(&reader, true) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getUint32(&reader, true) == uint32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getUint64 (little endian)", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getUint64(&reader, true) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getUint64(&reader, true) == uint64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getInt16 (little endian)", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getInt16(&reader, true) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getInt16(&reader, true) == int16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getInt32 (little endian)", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getInt32(&reader, true) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getInt32(&reader, true) == int32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getInt64 (little endian)", "[Reader]") {
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
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getInt64(&reader, true) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getInt64(&reader, true) == int64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getFloat32 (little endian)", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getFloat32(&reader, true) == -2.0f);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getFloat32(&reader, true) == float());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}

TEST_CASE("Reader_getFloat64 (little endian)", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 0, 0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  reader.data = view;

  CHECK(Reader_getFloat64(&reader, true) == -2.0);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError == Token_null());

  CHECK(Reader_getFloat64(&reader, true) == double());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError == Token_get("!out-of-bounds"));
}
} // namespace
