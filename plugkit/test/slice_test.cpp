#include "error.h"
#include "slice.h"
#include <catch.hpp>
#include <cstring>

using namespace plugkit;

namespace {

const Token outOfBoundError = Token_get("Out of bound");

TEST_CASE("Slice_length", "[Slice]") {
  char data[256];
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_length(view) == sizeof(data));
  Slice enptyView = {nullptr, nullptr};
  CHECK(Slice_length(enptyView) == 0);
}
} // namespace

TEST_CASE("Slice_slice", "[Slice]") {
  char data[256];
  Slice view = {data, data + sizeof(data)};
  view = Slice_slice(view, 1, 1281);
  CHECK(view.begin == data + 1);
  CHECK(view.end == data + 256);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_slice(view, 128, 133);
  CHECK(view.begin == data + 128);
  CHECK(view.end == data + 128 + 5);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_slice(view, 0, 128);
  CHECK(view.begin == data);
  CHECK(view.end == data + 128);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_slice(view, 1280, 2560);
  CHECK(view.begin == data + 256);
  CHECK(view.end == data + 256);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_slice(view, 0, 0);
  CHECK(view.begin == data);
  CHECK(view.end == data);
}

TEST_CASE("Slice_sliceAll", "[Slice]") {
  char data[256];
  Slice view = {data, data + sizeof(data)};
  view = Slice_sliceAll(view, 1);
  CHECK(view.begin == data + 1);
  CHECK(view.end == data + 256);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_sliceAll(view, 128);
  CHECK(view.begin == data + 128);
  CHECK(view.end == data + 256);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_sliceAll(view, 1280);
  CHECK(view.begin == data + 256);
  CHECK(view.end == data + 256);
}

TEST_CASE("Slice_getUint8", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(128)};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getUint8(view, 0, &err) == 128);
  CHECK(err.type == Token_null());

  CHECK(Slice_getUint8(view, 1, &err) == uint8_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getInt8", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-100};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getInt8(view, 0, &err) == -100);
  CHECK(err.type == Token_null());

  CHECK(Slice_getInt8(view, 1, &err) == int8_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getUint16", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 5};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getUint16(view, 0, false, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_getUint16(view, 1, false, &err) == uint16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getUint32", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getUint32(view, 0, false, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_getUint32(view, 3, false, &err) == uint32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getUint64", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 0, 0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getUint64(view, 0, false, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_getUint64(view, 7, false, &err) == uint64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getInt16", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getInt16(view, 0, false, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_getInt16(view, 1, false, &err) == int16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getInt32", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getInt32(view, 0, false, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_getInt32(view, 3, false, &err) == int32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getInt64", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getInt64(view, 0, false, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_getInt64(view, 7, false, &err) == int64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getFloat32", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-64, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getFloat32(view, 0, false, &err) == -2.0f);
  CHECK(err.type == Token_null());

  CHECK(Slice_getFloat32(view, 3, false, &err) == float());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getFloat64", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-64, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getFloat64(view, 0, false, &err) == -2.0);
  CHECK(err.type == Token_null());

  CHECK(Slice_getFloat64(view, 7, false, &err) == double());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getUint16 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getUint16(view, 0, true, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_getUint16(view, 1, true, &err) == uint16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getUint32 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getUint32(view, 0, true, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_getUint32(view, 3, true, &err) == uint32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getUint64 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getUint64(view, 0, true, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_getUint64(view, 7, true, &err) == uint64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getInt16 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getInt16(view, 0, true, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_getInt16(view, 1, true, &err) == int16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getInt32 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getInt32(view, 0, true, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_getInt32(view, 3, true, &err) == int32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getInt64 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0,
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255),
                 static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getInt64(view, 0, true, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_getInt64(view, 7, true, &err) == int64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getFloat32 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getFloat32(view, 0, true, &err) == -2.0f);
  CHECK(err.type == Token_null());

  CHECK(Slice_getFloat32(view, 3, true, &err) == float());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_getFloat64 (little endian)", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 0, 0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_getFloat64(view, 0, true, &err) == -2.0);
  CHECK(err.type == Token_null());

  CHECK(Slice_getFloat64(view, 7, true, &err) == double());
  CHECK(err.type == outOfBoundError);
}
