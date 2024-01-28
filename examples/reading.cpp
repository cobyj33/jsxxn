#include "json.h"

#include <string>
#include <iostream>

int main(int argc, char** argv) {
  json::JSON example = json::parse(R"x({ "name": "Peter", "date": { "month": 1, "day": 12, "year": 2023  }, "interests": ["running", "swimming", "climbing"] })x");
  std::string serialized = json::serialize(example);
  std::cout << serialized << std::endl;
  (void)argc; (void)argv;
  return 0;
}