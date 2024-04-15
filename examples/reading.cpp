#include "jsxxn.h"

#include <string>
#include <iostream>

int main(int argc, char** argv) {
  jsxxn::JSON example = jsxxn::parse(R"x({ "name": "Peter", "date": { "month": 1, "day": 12, "year": 2023  }, "interests": ["running", "swimming", "climbing"] })x");
  std::string serialized = jsxxn::prettify(example);
  std::cout << serialized << std::endl;
  (void)argc; (void)argv;
  return 0;
}