#include <catch.hpp>
#include "reader.h"

using namespace plugkit;

namespace {

const Token outOfBoundError = Token_get("Out of bound");

TEST_CASE("Reader_reset", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  CHECK(reader.slice.begin == nullptr);
  CHECK(reader.slice.end == nullptr);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == 0);
  CHECK(reader.lastError.type == Token_null());
}

TEST_CASE("Reader_slice", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[256];
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  Slice subview = Reader_slice(&reader, 1, 54);
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
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_sliceAll", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[256];
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  Slice subview = Reader_sliceAll(&reader, 12);
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
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readUint8", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(128)};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readUint8(&reader) == 128);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint8_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint8(&reader) == uint8_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint8_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readInt8", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-100};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readInt8(&reader) == -100);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int8_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt8(&reader) == int8_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int8_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readUint16BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 5};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readUint16BE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint16BE(&reader) == uint16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readUint32BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readUint32BE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint32BE(&reader) == uint32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readUint64BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 0, 0, 0, 0, 5};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readUint64BE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint64BE(&reader) == uint64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readInt16BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readInt16BE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt16BE(&reader) == int16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readInt32BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readInt32BE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt32BE(&reader) == int32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readInt64BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255), 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readInt64BE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt64BE(&reader) == int64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readFloat32BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-64, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readFloat32BE(&reader) == -2.0f);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat32BE(&reader) == float());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readFloat64BE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {-64, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readFloat64BE(&reader) == -2.0);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat64BE(&reader) == double());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readUint16LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readUint16LE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint16LE(&reader) == uint16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint16_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readUint32LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readUint32LE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint32LE(&reader) == uint32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint32_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readUint64LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {5, 0, 0, 0, 0, 0, 0, 0};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readUint64LE(&reader) == 5);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readUint64LE(&reader) == uint64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(uint64_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readInt16LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readInt16LE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt16LE(&reader) == int16_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int16_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readInt32LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, static_cast<char>(255), static_cast<char>(255),
                 static_cast<char>(255)};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readInt32LE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt32LE(&reader) == int32_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int32_t));
  CHECK(reader.lastError.type == outOfBoundError);
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
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readInt64LE(&reader) == -256);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readInt64LE(&reader) == int64_t());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(int64_t));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readFloat32LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readFloat32LE(&reader) == -2.0f);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat32LE(&reader) == float());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(float));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("Reader_readFloat64LE", "[Reader]") {
  Reader reader;
  Reader_reset(&reader);
  char data[] = {0, 0, 0, 0, 0, 0, 0, -64};
  Slice view = {data, data + sizeof(data)};
  reader.slice = view;

  CHECK(Reader_readFloat64LE(&reader) == -2.0);
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == Token_null());

  CHECK(Reader_readFloat64LE(&reader) == double());
  CHECK(reader.lastRange.begin == 0);
  CHECK(reader.lastRange.end == sizeof(double));
  CHECK(reader.lastError.type == outOfBoundError);
}

TEST_CASE("StreamReader_read", "[StreamReader]") {
  StreamReader *reader = StreamReader_create();
  char data[256] = {0};
  StreamReader_addSlice(reader, Slice{data, data + 16});
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});
  StreamReader_addSlice(reader, Slice{data, data + 16});
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});

  char buf[256] = {0};

  Slice slice = StreamReader_read(reader, buf, 16, 0);
  CHECK(slice.begin == data);
  CHECK(slice.end == data + 16);

  slice = StreamReader_read(reader, buf, 128, 0);
  CHECK(slice.begin == data);
  CHECK(slice.end == data + 128);

  slice = StreamReader_read(reader, buf, 256, 0);
  CHECK(slice.begin == data);
  CHECK(slice.end == data + 256);

  slice = StreamReader_read(reader, buf, 16, 16);
  CHECK(slice.begin == data + 16);
  CHECK(slice.end == data + 16 + 16);

  slice = StreamReader_read(reader, buf, 200, 32);
  CHECK(slice.begin == data + 32);
  CHECK(slice.end == data + 32 + 200);

  slice = StreamReader_read(reader, buf, sizeof(data) - 200, 200);
  CHECK(slice.begin == data + 200);
  CHECK(slice.end == data + sizeof(data));

  slice = StreamReader_read(reader, buf, 256, 12);
  CHECK(slice.begin == buf);
  CHECK(slice.end == buf + 256);

  StreamReader_destroy(reader);
}

TEST_CASE("StreamReader_search", "[StreamReader]") {
  StreamReader *reader = StreamReader_create();
  char data[256] = {'a', 'x', 'f', 'y'};
  char data2[] = {'a', 'x', 'd'};
  StreamReader_addSlice(reader, Slice{data2, data + sizeof(data2)});
  StreamReader_addSlice(reader, Slice{data, data + 50});
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});
  StreamReader_addSlice(reader, Slice{data2, data + sizeof(data2)});
  StreamReader_addSlice(reader, Slice{data, data + 16});
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});

  size_t offset = 0;
  Range range = StreamReader_search(reader, "xd", 2, &offset);
  CHECK(range.begin == 1);
  CHECK(range.end == 3);
  range = StreamReader_search(reader, "xd", 2, &offset);
  CHECK(range.begin == 322);
  CHECK(range.end == 324);
  range = StreamReader_search(reader, "xfy", 3, &offset);
  CHECK(range.begin == 353);
  CHECK(range.end == 356);
  range = StreamReader_search(reader, "xfy", 3, &offset);
  CHECK(range.begin == 0);
  CHECK(range.end == 0);
}
}
