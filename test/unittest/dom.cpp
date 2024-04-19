#include "jsxxn.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("dom") {

  SECTION("Array Push Back on uninitialized JSON Value") {
    jsxxn::JSON json(jsxxn::JSXXNValueType::ARRAY);
    REQUIRE_NOTHROW(json.push_back(5));
    REQUIRE(json.size() == 1);
  }

  SECTION("Array Push Back") {
    jsxxn::JSON json(jsxxn::JSXXNValueType::ARRAY);
    REQUIRE_NOTHROW(json.push_back(5));
    REQUIRE(json.size() == 1);
  }

  SECTION("Default to double") {
    jsxxn::JSON json(jsxxn::JSONValueType::NUMBER);
    REQUIRE(json.type() == jsxxn::JSONValueType::NUMBER);
    REQUIRE(json.xtype() == jsxxn::JSXXNValueType::DOUBLE);
  }

  SECTION("Flat Object Construction") {
    jsxxn::JSON arr(jsxxn::JSONValueType::ARRAY);

    arr.push_back(5);
    arr.push_back(3);
    REQUIRE(arr.at(1).equals_deep(3));
  }

  SECTION("Flat Array Construction") {


  }

}