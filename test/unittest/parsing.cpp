#include "jsxxn.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("trivial", "[parsing]") {
  SECTION("Number Parsing") {
    jsxxn::JSON num = jsxxn::parse("10");
    REQUIRE(num.type() == jsxxn::JSONValueType::NUMBER);
  }
}