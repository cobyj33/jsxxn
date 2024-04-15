#include "jsxxn.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("serializing") {
  
  SECTION("trivial") {
    REQUIRE(json::serialize(json::parse("{}")) == json::serialize(json::JSONObject()));
  }
}