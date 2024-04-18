#include "jsxxn.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {


  // Source: https://pokeapi.co/api/v2/pokemon/ditto
  jsxxn::JSON json = jsxxn::parse(R"([ { "name": 3 }, { "age": 4 }, [ 3, 5, 8 ], "String" ])");

  jsxxn::JSON first(0.0f);
  first["injected"] = "hello";
  std::cout << jsxxn::prettify(json) << std::endl;

  return EXIT_SUCCESS;
}