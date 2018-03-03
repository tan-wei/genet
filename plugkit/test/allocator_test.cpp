#include "allocator.hpp"
#include <catch.hpp>

using namespace plugkit;

namespace {

struct Data {
  Data(uint64_t a) : a(a) {}
  uint64_t a;
};

TEST_CASE("BlockAllocator", "[Allocator]") {
  RootAllocator root;
  BlockAllocator<Data> allocator(&root);
  CHECK(allocator.allocUninitialized(0) == nullptr);
  CHECK(allocator.allocUninitialized(1024) == nullptr);
  for (int i = 1; i < 1024; ++i) {
    CHECK(allocator.allocUninitialized(i) != nullptr);
  }
}

TEST_CASE("BlockVector", "[Allocator]") {
  RootAllocator root;
  BlockAllocator<Data> allocator(&root);

  BlockVector<Data> vec1;
  BlockVector<Data> vec2;
  BlockVector<Data> vec3;

  CHECK(vec1.size() == 0);
  for (int i = 0; i < 512; ++i) {
    vec1.emplace_back(&allocator, i);
  }
  CHECK(vec1.size() == 512);
  for (size_t i = 0; i < vec1.size(); ++i) {
    CHECK(vec1.data()[i].a == i);
  }

  CHECK(vec2.size() == 0);
  for (int i = 0; i < 1000; ++i) {
    vec2.emplace_back(&allocator, i);
  }
  CHECK(vec2.size() == 1000);
  for (size_t i = 0; i < vec2.size(); ++i) {
    CHECK(vec2.data()[i].a == i);
  }

  CHECK(vec3.size() == 0);
  for (int i = 0; i < 1022; ++i) {
    vec3.emplace_back(&allocator, i);
  }
  CHECK(vec3.size() == 1022);
  for (size_t i = 0; i < vec3.size(); ++i) {
    CHECK(vec3.data()[i].a == i);
  }

  int begin = vec1.size();
  for (int i = begin; i < begin + 128; ++i) {
    vec1.emplace_back(&allocator, i);
  }
  CHECK(vec1.size() == 640);
  for (size_t i = 0; i < vec1.size(); ++i) {
    CHECK(vec1.data()[i].a == i);
  }
}

} // namespace
