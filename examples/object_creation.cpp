#include "jsxxn.h"

#include <string>
#include <iostream>

int main(int argc, char** argv) {
  (void)argc; (void)argv;

  jsxxn::JSONObject example_obj = {
    {"name", "Christmas"},
    {"date", jsxxn::JSONObject({
      {"day", 25},
      {"month", "December"},
      {"year", 2023}
    })},
    {"description", "The most wonderful time of the year"}
  };
  
  std::cout << jsxxn::serialize(example_obj) << std::endl;
  std::cout << std::endl;
  // a round trip parse to prove that parsing and reserializing are inverse operations.
  std::cout << jsxxn::serialize(jsxxn::parse(jsxxn::serialize(example_obj))) << std::endl;
  return 0;
}