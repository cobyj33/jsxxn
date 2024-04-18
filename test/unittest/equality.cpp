#include "jsxxn.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("json equality") {
  SECTION("trivial") {
    SECTION("string") {
      jsxxn::JSON str1 = "This is a string";
      jsxxn::JSON str2 = "This is a string";
      REQUIRE(str1.equals_deep(str2));
    }

    SECTION("positive integer") {
      jsxxn::JSON int1 = 5;
      jsxxn::JSON int2 = 5;
      REQUIRE(int1.equals_deep(int2));
    }

    SECTION("negative integer") {
      jsxxn::JSON int1 = -5;
      jsxxn::JSON int2 = -5;
      REQUIRE(int1.equals_deep(int2));
    }

    SECTION("nullptr") {
      jsxxn::JSON nullptr1 = nullptr;
      jsxxn::JSON nullptr2 = nullptr;
      REQUIRE(nullptr1.equals_deep(nullptr2));
      REQUIRE(jsxxn::JSON(nullptr).equals_deep(jsxxn::JSON(nullptr)));
    }

    SECTION("boolean") {
      jsxxn::JSON True = true;
      jsxxn::JSON False = false;
      REQUIRE_FALSE(True.equals_deep(False));
      REQUIRE(True.equals_deep(True));
      REQUIRE(False.equals_deep(False));
    }

    SECTION("Empty Array") {
      jsxxn::JSON arr1(jsxxn::JSONValueType::ARRAY);
      jsxxn::JSON arr2(jsxxn::JSONValueType::ARRAY);
      REQUIRE(arr1.equals_deep(arr2));
      REQUIRE(arr1.equals_deep(arr1));
      REQUIRE(arr2.equals_deep(arr2));
    }
  }
}