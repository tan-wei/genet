#include "token.h"
#include <catch.hpp>

using namespace plugkit;

namespace {

TEST_CASE("Token_null") {
  CHECK(Token_null() == 0);
  CHECK(Token_null() == Token_null());
}

TEST_CASE("Token_get", "[Token]") {
  CHECK(Token_get(nullptr) == Token_null());
  CHECK(Token_get("") == Token_null());
  CHECK(Token_get("a") == Token_get("a"));
  CHECK(Token_get("eth") == Token_get("eth"));
  CHECK(Token_get("[eth]") == Token_get("[eth]"));
  CHECK(Token_get("ipv4") == Token_get("ipv4"));
  CHECK(Token_get("[ipv4]") == Token_get("[ipv4]"));
  CHECK(Token_get("dd31817d-1501-4b2b-bcf6-d02e148d3ab9") ==
        Token_get("dd31817d-1501-4b2b-bcf6-d02e148d3ab9"));
  CHECK(Token_get("187b3d73-1f2e-41a8-a84c-c47975d99994") ==
        Token_get("187b3d73-1f2e-41a8-a84c-c47975d99994"));
  CHECK(Token_get("cdfbbd84-4a9b-4db1-85d7-895648244607") ==
        Token_get("cdfbbd84-4a9b-4db1-85d7-895648244607"));
  CHECK(Token_get("8dac2cdd-576e-409f-bfd6-577ba599ce03") ==
        Token_get("8dac2cdd-576e-409f-bfd6-577ba599ce03"));
  CHECK(Token_get("bb16dd92-75f0-4253-8e05-c6f4d96fc264") ==
        Token_get("bb16dd92-75f0-4253-8e05-c6f4d96fc264"));
  CHECK(Token_get("04da026d-04e4-4f16-8f6c-19501e17f951") ==
        Token_get("04da026d-04e4-4f16-8f6c-19501e17f951"));
  CHECK(Token_get("b6be94b2-89e3-4ad5-9643-299072fc41b7") ==
        Token_get("b6be94b2-89e3-4ad5-9643-299072fc41b7"));
  CHECK(Token_get("a3c776fd-66ee-4428-995e-f0bb031c1bc9") ==
        Token_get("a3c776fd-66ee-4428-995e-f0bb031c1bc9"));
  CHECK(Token_get("301a61ec-1461-4972-86cb-2b5fd56cc8cd") ==
        Token_get("301a61ec-1461-4972-86cb-2b5fd56cc8cd"));
  CHECK(Token_get("9bbb35e6-cde1-419b-8d7b-ea04c87e7320") ==
        Token_get("9bbb35e6-cde1-419b-8d7b-ea04c87e7320"));
  CHECK(Token_get("ab") != Token_get("ba"));
}

TEST_CASE("Token_string", "[Token]") {
  CHECK(Token_get(Token_string(Token_get(nullptr))) == Token_get(nullptr));
  CHECK(Token_get(Token_string(Token_get(""))) == Token_get(""));
  CHECK(Token_get(Token_string(Token_get("a"))) == Token_get("a"));
  CHECK(Token_get(Token_string(Token_get("eth"))) == Token_get("eth"));
  CHECK(Token_get(Token_string(Token_get("[eth]"))) == Token_get("[eth]"));
  CHECK(Token_get(Token_string(Token_get("ipv4"))) == Token_get("ipv4"));
  CHECK(Token_get(Token_string(Token_get("[ipv4]"))) == Token_get("[ipv4]"));

  CHECK(Token_get(
            Token_string(Token_get("9275ac5c-c7b5-4657-935b-612fd7b28d02"))) ==
        Token_get("9275ac5c-c7b5-4657-935b-612fd7b28d02"));
  CHECK(Token_get(
            Token_string(Token_get("310cf0e8-3764-467e-b3dd-048cc29ca680"))) ==
        Token_get("310cf0e8-3764-467e-b3dd-048cc29ca680"));
  CHECK(Token_get(
            Token_string(Token_get("cd8d6a65-619b-4246-97f1-e4fcbff42a45"))) ==
        Token_get("cd8d6a65-619b-4246-97f1-e4fcbff42a45"));
  CHECK(Token_get(
            Token_string(Token_get("a529d95e-3e89-4486-a17a-2799750b847a"))) ==
        Token_get("a529d95e-3e89-4486-a17a-2799750b847a"));
  CHECK(Token_get(
            Token_string(Token_get("ec2d0ee1-710b-4269-863b-38b4c06a2cd7"))) ==
        Token_get("ec2d0ee1-710b-4269-863b-38b4c06a2cd7"));
  CHECK(Token_get(
            Token_string(Token_get("5f4e4f06-819d-4e71-a48d-26d86e28ad01"))) ==
        Token_get("5f4e4f06-819d-4e71-a48d-26d86e28ad01"));
  CHECK(Token_get(
            Token_string(Token_get("f0b69ffe-98cb-45e3-bea0-9904a2fd1a52"))) ==
        Token_get("f0b69ffe-98cb-45e3-bea0-9904a2fd1a52"));
  CHECK(Token_get(
            Token_string(Token_get("9a0ec074-967e-40e5-a93e-d7a9b4249704"))) ==
        Token_get("9a0ec074-967e-40e5-a93e-d7a9b4249704"));
  CHECK(Token_get(
            Token_string(Token_get("0f73db21-11f6-46e0-948f-df30462df37f"))) ==
        Token_get("0f73db21-11f6-46e0-948f-df30462df37f"));
  CHECK(Token_get(
            Token_string(Token_get("7a250b5e-0cb3-4987-81f6-1a8bb5f26704"))) ==
        Token_get("7a250b5e-0cb3-4987-81f6-1a8bb5f26704"));
}

TEST_CASE("Token_join", "[Token]") {
  CHECK(Token_join(Token_null(), nullptr) == Token_null());
  CHECK(Token_join(Token_null(), "") == Token_null());
  CHECK(Token_join(Token_get("eth"), ".dst") == Token_get("eth.dst"));
  CHECK(Token_join(Token_null(), ".dst") == Token_get(".dst"));
  CHECK(Token_join(Token_get("eth"), "") == Token_get("eth"));
  CHECK(Token_join(Token_get("eth"), nullptr) == Token_get("eth"));
  CHECK(Token_join(Token_get("0f73db21-11f6-"), "4987-81f6-1a8bb5f26704") ==
        Token_get("0f73db21-11f6-4987-81f6-1a8bb5f26704"));
}

} // namespace
