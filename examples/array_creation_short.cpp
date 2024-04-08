#include "json.h"

#include <string>
#include <iostream>

// Design Note:

// While I would like to allow json::JSONArray and json::JSONObject to not
// be present when using a shorthand initializer_list expression like this,
// I can foresee unwanted runtime behavior if we interpret {"string", value}
// pairs as objects, as they are ambiguous when compared to 2-length arrays.

// This is why the JSON class does not have std::initializer_list constructors
// or std::initializer_list assignment operators, as std::initializer_list<JSON>
// and std::initializer_list<std::pair<const std::string, JSON>> are ambiguous

// nholmann/json seems to just determine this at runtime by checking
// if every object is a pair which starts with a string, so 
//
// {
//  {"name", "Jacoby"},
//  {"age", 18}
// }
// is interpreted as an object, but
// {
//  {"name", "Jacoby"},
//  {"age", 18},
//  {3, 5}
// }
// is a group of 2-length arrays.
// 
// While it is nicer syntax, I'm not so sold on not as much compiler support
// for detecting errors. This design choice is definitely up to debate though,
// as it might just be a non-issue that I'm over-exaggerating


int main(int argc, char** argv) {
  (void)argc; (void)argv;

  // note that I have declared arr as a json::JSONArray
  json::JSONArray arr = {
    1,
    2,
    "words",
    nullptr,
    -12,
    std::string_view("string_view"), // this would just be implicitly casted to std::string
    json::JSONArray({ "nested", "initializer", "list", "inside"}),
    json::JSONObject({
      {"object", "test"},
      {"set", "of keys"},
      {"types", 2},
      {"null", nullptr},
      {"test", false}
    })
  };

  std::cout << json::serialize(arr) << std::endl;

  // This, however, does not compile when we declare arr to be json::JSON
  // instead of json::JSONArray, as the std::initializer_list constructor 
  // on json::JSON does not exist. You can try uncommenting and building
  // again to check
  //
  // json::JSON arr = {
  //   1,
  //   2,
  //   "words",
  //   nullptr,
  //   -12,
  //   std::string_view("string_view"),
  //   json::JSONArray({ "nested", "initializer", "list", "inside"}),
  //   json::JSONObject({
  //     {"object", "test"},
  //     {"set", "of keys"},
  //     {"types", 2},
  //     {"null", nullptr},
  //     {"test", false}
  //   })
  // };

  // This does compile, as the json::JSONArray constructor after
  // assignment makes explicit what the initializer list represents
  json::JSON arrJSON = json::JSONArray({
    1,
    2,
    "words",
    nullptr,
    -12,
    std::string_view("string_view"),
    json::JSONArray({ "nested", "initializer", "list", "inside"}),
    json::JSONObject({
      {"object", "test"},
      {"set", "of keys"},
      {"types", 2},
      {"null", nullptr},
      {"test", false}
    })
  });

  std::cout << json::serialize(arrJSON) << std::endl;

  return 0;
}