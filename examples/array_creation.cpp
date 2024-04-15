#include "jsxxn.h"

#include <string>
#include <iostream>

int main(int argc, char** argv) {
  (void)argc; (void)argv;

  // a more verbose way of declaring a JSON object through STL methods
  // A shorthand is shown in array_creation_short.cpp

  // note that jsxxn::JSONArray is just a typedef for std::vector<JSON>
  // and jsxxn::JSONObject is just a typedef for std::map<std::string, JSON>, 
  // so all stl functions can be used.

  jsxxn::JSONArray arr;
  arr.push_back(1);
  arr.push_back(2);
  arr.push_back("words");
  arr.push_back(nullptr);
  arr.push_back(-12);
  arr.push_back(std::string_view("string_view"));
  arr.insert(arr.begin() + 3, jsxxn::JSONArray({ "nested", "initializer", "list", "inside" }));

  // note that this line adds the array flatly into the top level array. It may
  // seem unintuitive, but the example above inserts an array into the index,
  // while this example inserts the values in the initializer list flatly.
  arr.insert(arr.begin() + 3, { "inserted", "initializer", "list", "inside" });


  jsxxn::JSONObject obj;
  obj["name"] = "first object";
  obj["object"] = "test";
  obj["set"] = "of keys";
  obj["types"] = 2;
  obj["null"] = nullptr;

  arr.push_back(obj);

  arr.push_back(jsxxn::JSONObject({
    {"here is", 5},
    {"another object", 113.55},
    {"with a given set of keys", jsxxn::JSONObject({
      {"that can nest", jsxxn::JSONArray({ "data", "quite", nullptr, "nicely", 101, "together"})}
    })}
  }));

  jsxxn::JSON five = 5;
  obj["note that this key will not show up in the first object"] = "although we are using the same stack variable";
  obj["but will show up"] = "as a separate copied object in the array";

  arr.push_back(five); 
  arr.push_back(obj);

  // round trip, no particular reason but to show that it works
  std::cout << jsxxn::prettify(jsxxn::parse(jsxxn::prettify(arr))) << std::endl;

  return 0;
}