#include <catch.hpp>
#include "stream_reader.h"

using namespace plugkit;

namespace {

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
  StreamReader_addSlice(reader, Slice{data2, data2 + sizeof(data2)});
  StreamReader_addSlice(reader, Slice{data, data + 50});
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});
  StreamReader_addSlice(reader, Slice{data2, data2 + sizeof(data2)});
  StreamReader_addSlice(reader, Slice{data, data + 16});
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});

  size_t offset = 0;
  Range range = StreamReader_search(reader, "xd", 2, offset);
  CHECK(range.begin == 1);
  CHECK(range.end == 3);
  offset = range.end;
  range = StreamReader_search(reader, "xd", 2, offset);
  CHECK(range.begin == 294);
  CHECK(range.end == 296);
  offset = range.end;
  range = StreamReader_search(reader, "xfy", 3, offset);
  CHECK(range.begin == 297);
  CHECK(range.end == 300);
  offset = range.end;
  range = StreamReader_search(reader, "xfy", 3, StreamReader_length(reader));
  CHECK(range.begin == 0);
  CHECK(range.end == 0);

  StreamReader_destroy(reader);
}

TEST_CASE("StreamReader_length", "[StreamReader]") {
  StreamReader *reader = StreamReader_create();
  CHECK(StreamReader_length(reader) == 0);

  char data[256] = {0};
  StreamReader_addSlice(reader, Slice{data, data + 16});
  CHECK(StreamReader_length(reader) == 16);
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  CHECK(StreamReader_length(reader) == 100);
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});
  CHECK(StreamReader_length(reader) == 256);
  StreamReader_addSlice(reader, Slice{data, data + 16});
  CHECK(StreamReader_length(reader) == 272);
  StreamReader_addSlice(reader, Slice{data + 16, data + 100});
  CHECK(StreamReader_length(reader) == 356);
  StreamReader_addSlice(reader, Slice{data + 100, data + sizeof(data)});
  CHECK(StreamReader_length(reader) == 512);

  StreamReader_destroy(reader);
}
}
