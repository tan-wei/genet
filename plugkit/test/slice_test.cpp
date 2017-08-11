#include <catch.hpp>
#include <cstring>
#include "slice.h"
#include "error.h"

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
}

TEST_CASE("Slice_slice", "[Slice]") {
  char data[256];
  Slice view = {data, data + sizeof(data)};
  view = Slice_slice(view, 1, 1280);
  CHECK(view.begin == data + 1);
  CHECK(view.end == data + 256);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_slice(view, 128, 5);
  CHECK(view.begin == data + 128);
  CHECK(view.end == data + 128 + 5);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_slice(view, 0, 128);
  CHECK(view.begin == data);
  CHECK(view.end == data + 128);

  view.begin = data;
  view.end = data + sizeof(data);
  view = Slice_slice(view, 1280, 1280);
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

TEST_CASE("Slice_readUint8", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(128)};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readUint8(view, 0, &err) == 128);
  CHECK(err.type == Token_null());

  CHECK(Slice_readUint8(view, 1, &err) == uint8_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readInt8", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-100};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readInt8(view, 0, &err) == -100);
  CHECK(err.type == Token_null());

  CHECK(Slice_readInt8(view, 1, &err) == int8_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readUint16BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 5};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readUint16BE(view, 0, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_readUint16BE(view, 1, &err) == uint16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readUint32BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readUint32BE(view, 0, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_readUint32BE(view, 3, &err) == uint32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readUint64BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 0, 0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readUint64BE(view, 0, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_readUint64BE(view, 7, &err) == uint64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readInt16BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readInt16BE(view, 0, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_readInt16BE(view, 1, &err) == int16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readInt32BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readInt32BE(view, 0, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_readInt32BE(view, 3, &err) == int32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readInt64BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readInt64BE(view, 0, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_readInt64BE(view, 7, &err) == int64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readFloat32BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-64, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readFloat32BE(view, 0, &err) == -2.0f);
  CHECK(err.type == Token_null());

  CHECK(Slice_readFloat32BE(view, 3, &err) == float());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readFloat64BE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-64, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readFloat64BE(view, 0, &err) == -2.0);
  CHECK(err.type == Token_null());

  CHECK(Slice_readFloat64BE(view, 7, &err) == double());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readUint16LE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readUint16LE(view, 0, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_readUint16LE(view, 1, &err) == uint16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readUint32LE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readUint32LE(view, 0, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_readUint32LE(view, 3, &err) == uint32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readUint64LE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readUint64LE(view, 0, &err) == 5);
  CHECK(err.type == Token_null());

  CHECK(Slice_readUint64LE(view, 7, &err) == uint64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readInt16LE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readInt16LE(view, 0, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_readInt16LE(view, 1, &err) == int16_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readInt32LE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readInt32LE(view, 0, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_readInt32LE(view, 3, &err) == int32_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readInt64LE", "[Slice]") {
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
  CHECK(Slice_readInt64LE(view, 0, &err) == -256);
  CHECK(err.type == Token_null());

  CHECK(Slice_readInt64LE(view, 7, &err) == int64_t());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readFloat32LE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readFloat32LE(view, 0, &err) == -2.0f);
  CHECK(err.type == Token_null());

  CHECK(Slice_readFloat32LE(view, 3, &err) == float());
  CHECK(err.type == outOfBoundError);
}

TEST_CASE("Slice_readFloat64LE", "[Slice]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 0, 0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  CHECK(Slice_readFloat64LE(view, 0, &err) == -2.0);
  CHECK(err.type == Token_null());

  CHECK(Slice_readFloat64LE(view, 7, &err) == double());
  CHECK(err.type == outOfBoundError);
}
