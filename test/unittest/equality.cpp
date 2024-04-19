#include "jsxxn.h"

#include <catch2/catch_test_macros.hpp>

#define REQUIRE_EQUALS_COMBOS(json1, json2) do { \
    REQUIRE((json1).equals_deep(json2)); \
    REQUIRE((json2).equals_deep(json1)); \
    REQUIRE((json1).equals_deep(json1)); \
    REQUIRE((json2).equals_deep(json2)); \
  } while(0)

#define REQUIRE_FALSE_EQUALS_COMBOS(json1, json2) do { \
    REQUIRE(!((json1).equals_deep(json2))); \
    REQUIRE(!((json2).equals_deep(json1))); \
    REQUIRE((json1).equals_deep(json1)); \
    REQUIRE((json2).equals_deep(json2)); \
  } while(0)

TEST_CASE("json equality") {
  SECTION("trivial") {
    SECTION("string") {
      jsxxn::JSON str1 = "This is a string";
      jsxxn::JSON str2 = "This is a string";
      REQUIRE_EQUALS_COMBOS(str1, str2);
    }

    SECTION("positive integer") {
      jsxxn::JSON int1 = 5;
      jsxxn::JSON int2 = 5;
      REQUIRE_EQUALS_COMBOS(int1, int2);
    }

    SECTION("negative integer") {
      jsxxn::JSON int1 = -5;
      jsxxn::JSON int2 = -5;
      REQUIRE_EQUALS_COMBOS(int1, int2);
    }

    SECTION("nullptr") {
      jsxxn::JSON nullptr1 = nullptr;
      jsxxn::JSON nullptr2 = nullptr;
      REQUIRE_EQUALS_COMBOS(nullptr1, nullptr2);
      REQUIRE_EQUALS_COMBOS(jsxxn::JSON(nullptr), (jsxxn::JSON(nullptr)));
    }

    SECTION("boolean") {
      jsxxn::JSON True = true;
      jsxxn::JSON False = false;
      REQUIRE_FALSE_EQUALS_COMBOS(True, False);
    }

    SECTION("Empty Array") {
      jsxxn::JSON arr1(jsxxn::JSONValueType::ARRAY);
      jsxxn::JSON arr2(jsxxn::JSONValueType::ARRAY);
      REQUIRE(arr1.equals_deep(arr2));
      REQUIRE(arr1.equals_deep(arr1));
      REQUIRE(arr2.equals_deep(arr2));
      REQUIRE_EQUALS_COMBOS(arr1, arr2);
    }

    SECTION("Empty Object") {
      jsxxn::JSON obj1(jsxxn::JSONValueType::OBJECT);
      jsxxn::JSON obj2(jsxxn::JSONValueType::OBJECT);
      REQUIRE_EQUALS_COMBOS(obj1, obj2);
    }
  } // SECTION trivial

}