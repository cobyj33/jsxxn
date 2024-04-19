#include "jsxxn.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("serializing") {
  
  SECTION("trivial") {
    REQUIRE(jsxxn::prettify(jsxxn::parse("{}")) == jsxxn::prettify(jsxxn::JSONObject()));
  }
}