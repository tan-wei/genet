#include <catch.hpp>
#include "view.h"
#include "error.h"

using namespace plugkit;

namespace {

Token outOfBoundError() {
  static const Token token = Token_get("Out of bound");
  return token;
}

TEST_CASE("View_length", "[View]") {
  char data[256];
  View view = {data, data + sizeof(data)};
  CHECK(View_length(view) == sizeof(data));
  View enptyView = {nullptr, nullptr};
  CHECK(View_length(enptyView) == 0);
}
}

TEST_CASE("View_slice", "[View]") {
  char data[256];
  View view = {data, data + sizeof(data)};
  view = View_slice(view, 1, 1280);
  CHECK(view.begin == data + 1);
  CHECK(view.end == data + 256);

  view.begin = data;
  view.end = data + sizeof(data);
  view = View_slice(view, 128, 5);
  CHECK(view.begin == data + 128);
  CHECK(view.end == data + 128 + 5);

  view.begin = data;
  view.end = data + sizeof(data);
  view = View_slice(view, 0, 128);
  CHECK(view.begin == data);
  CHECK(view.end == data + 128);

  view.begin = data;
  view.end = data + sizeof(data);
  view = View_slice(view, 1280, 1280);
  CHECK(view.begin == data + 256);
  CHECK(view.end == data + 256);

  view.begin = data;
  view.end = data + sizeof(data);
  view = View_slice(view, 0, 0);
  CHECK(view.begin == data);
  CHECK(view.end == data);
}

TEST_CASE("View_readUint8", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(128)};
  View view = {data, data + sizeof(data)};
  CHECK(View_readUint8(view, &err) == 128);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data;
  CHECK(View_readUint8(view, &err) == uint8_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readInt8", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-100};
  View view = {data, data + sizeof(data)};
  CHECK(View_readInt8(view, &err) == -100);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data;
  CHECK(View_readInt8(view, &err) == int8_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readUint16BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 5};
  View view = {data, data + sizeof(data)};
  CHECK(View_readUint16BE(view, &err) == 5);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 1;
  CHECK(View_readUint16BE(view, &err) == uint16_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readUint32BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 5};
  View view = {data, data + sizeof(data)};
  CHECK(View_readUint32BE(view, &err) == 5);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 3;
  CHECK(View_readUint32BE(view, &err) == uint32_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readUint64BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 0, 0, 0, 0, 5};
  View view = {data, data + sizeof(data)};
  CHECK(View_readUint64BE(view, &err) == 5);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 7;
  CHECK(View_readUint64BE(view, &err) == uint64_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readInt16BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readInt16BE(view, &err) == -256);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 1;
  CHECK(View_readInt16BE(view, &err) == int16_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readInt32BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readInt32BE(view, &err) == -256);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 3;
  CHECK(View_readInt32BE(view, &err) == int32_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readInt64BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readInt64BE(view, &err) == -256);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 7;
  CHECK(View_readInt64BE(view, &err) == int64_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readFloat32BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-64, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readFloat32BE(view, &err) == -2.0f);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 3;
  CHECK(View_readFloat32BE(view, &err) == float());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readFloat64BE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {-64, 0, 0, 0, 0, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readFloat64BE(view, &err) == -2.0);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 7;
  CHECK(View_readFloat64BE(view, &err) == double());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readUint16LE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readUint16LE(view, &err) == 5);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 1;
  CHECK(View_readUint16LE(view, &err) == uint16_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readUint32LE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readUint32LE(view, &err) == 5);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 3;
  CHECK(View_readUint32LE(view, &err) == uint32_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readUint64LE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {5, 0, 0, 0, 0, 0, 0, 0};
  View view = {data, data + sizeof(data)};
  CHECK(View_readUint64LE(view, &err) == 5);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 7;
  CHECK(View_readUint64LE(view, &err) == uint64_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readInt16LE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, static_cast<char>(255)};
  View view = {data, data + sizeof(data)};
  CHECK(View_readInt16LE(view, &err) == -256);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 1;
  CHECK(View_readInt16LE(view, &err) == int16_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readInt32LE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255)};
  View view = {data, data + sizeof(data)};
  CHECK(View_readInt32LE(view, &err) == -256);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 3;
  CHECK(View_readInt32LE(view, &err) == int32_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readInt64LE", "[View]") {
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
  View view = {data, data + sizeof(data)};
  CHECK(View_readInt64LE(view, &err) == -256);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 7;
  CHECK(View_readInt64LE(view, &err) == int64_t());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readFloat32LE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, -64};
  View view = {data, data + sizeof(data)};
  CHECK(View_readFloat32LE(view, &err) == -2.0f);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 3;
  CHECK(View_readFloat32LE(view, &err) == float());
  CHECK(err.type == outOfBoundError());
}

TEST_CASE("View_readFloat64LE", "[View]") {
  Error err;
  std::memset(&err, 0, sizeof(Error));
  char data[] = {0, 0, 0, 0, 0, 0, 0, -64};
  View view = {data, data + sizeof(data)};
  CHECK(View_readFloat64LE(view, &err) == -2.0);
  CHECK(err.type == Token_null());

  view.begin = data;
  view.end = data + 7;
  CHECK(View_readFloat64LE(view, &err) == double());
  CHECK(err.type == outOfBoundError());
}
