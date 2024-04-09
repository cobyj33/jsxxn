#include "json.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("trivial", "[parsing]") {
  SECTION("Number Parsing") {
    json::JSON num = json::parse("10");
    REQUIRE(num.type() == json::JSONValueType::NUMBER);
  }
}