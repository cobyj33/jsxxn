#include "json.h"

#include <string>
#include <iostream>

int main(int argc, char** argv) {
  (void)argc; (void)argv;

  json::JSONObject example_obj = {
    {"name", "Christmas"},
    {"date", json::JSONObject({
      {"day", 25},
      {"month", "December"},
      {"year", 2023}
    })},
    {"description", "The most wonderful time of the year"}
  };

  example_obj.insert(std::initializer_list<json::JSONObject::value_type>)

  std::cout << json::serialize(example_obj) << std::endl;
  std::cout << std::endl;
  std::cout << json::serialize(json::parse(json::serialize(example_obj))) << std::endl;
  return 0;
}