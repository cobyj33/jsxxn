#include "jsxxn.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("dom") {

  SECTION("Array Push Back on uninitialized JSON Value") {
    jsxxn::JSON json; // don't do this (at least as of now!)
    REQUIRE_THROWS(json.push_back(5));
  }

  SECTION("Array Push Back") {
    jsxxn::JSON json(jsxxn::JSXXNValueType::ARRAY);
    REQUIRE_NOTHROW(json.push_back(5));
    REQUIRE(json.size() == 1);
    REQUIRE_NOTHROW(json.push_back(jsxxn::JSONArray({ 5, 3, jsxxn::JSONArray({ "inner", "array" })})));
    REQUIRE(json.size() == 2);
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
    REQUIRE(arr.at(0).equals_deep(5));
    REQUIRE(arr.at(1).equals_deep(3));
  }

  SECTION("Value switching") {
    // in actual code, you definitely should not just switch between container
    // and literal types on a whim for performance reasons.

    jsxxn::JSON val(jsxxn::JSONValueType::ARRAY);
    val = 5;
    REQUIRE(val.xtype() == jsxxn::JSXXNValueType::SINTEGER);
    val = "a little string";
    REQUIRE(val.type() == jsxxn::JSONValueType::STRING);

    val = jsxxn::JSONObject({
      {"key", "value"},
      {"key2", jsxxn::JSONObject({
        { "key", "value" }
      }) 
    } });
    
    REQUIRE(val.type() == jsxxn::JSONValueType::OBJECT);
    val = 5.5;
    REQUIRE(val.xtype() == jsxxn::JSXXNValueType::DOUBLE);
    val = true;
    REQUIRE(val.xtype() == jsxxn::JSXXNValueType::BOOLEAN);
    val = nullptr;
    REQUIRE(val.xtype() == jsxxn::JSXXNValueType::NULLPTR);

  }

}