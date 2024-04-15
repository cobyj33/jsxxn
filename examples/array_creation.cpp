#include "jsxxn.h"

#include <string>
#include <iostream>

int main(int argc, char** argv) {
  (void)argc; (void)argv;

  // a more verbose way of declaring a JSON object through STL methods
  // A shorthand is shown in array_creation_short.cpp

  // note that jsxxn::JSONArray is just a typedef for std::vector<JSON>
  // and jsxxn::JSONObject is just a typedef for std::map<std::string, JSON>, 
  // so they can use all of the STL functions that you're already used to

  jsxxn::JSONArray arr;
  arr.push_back(1);
  arr.push_back(2);
  arr.push_back("words");
  arr.push_back(nullptr);
  arr.push_back(-12);
  arr.push_back(std::string_view("string_view"));
  arr.insert(arr.begin() + 3, jsxxn::JSONArray({ "nested", "initializer", "list", "inside" }));

  // note that this line adds the array flatly into the top level array. I don't
  // really know why but it does :(
  arr.insert(arr.begin() + 3, { "inserted", "initializer", "list", "inside" });


  jsxxn::JSONObject obj;
  obj["object"] = "test";
  obj["set"] = "of keys";
  obj["types"] = 2;
  obj["null"] = nullptr;

  arr.push_back(obj);

  std::cout << jsxxn::serialize(jsxxn::parse(jsxxn::serialize(arr))) << std::endl;

  return 0;
}