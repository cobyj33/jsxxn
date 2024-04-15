#include "jsxxn.h"

#include <iostream>


int main(int argc, char** argv) {
  (void)argc; (void)argv;

  // note that it would be much nicer to use a raw string when writing literals
  // directly into source code, like as seen here, rather than escaping every
  // single double quote

  // Raw string literal documentation: https://en.cppreference.com/w/cpp/language/string_literal
  
  // The json in this given section is publicly available from 
  // RFC 8259 (The JavaScript Object Notation (JSON) Data Interchange Format)
  // section 13 (https://www.rfc-editor.org/rfc/rfc8259#section-13)
  
  jsxxn::JSON parsed = jsxxn::parse(R"([ 
    {
      "precision": "zip",
      "Latitude":  37.7668,
      "Longitude": -122.3959,
      "Address":   "",
      "City":      "SAN FRANCISCO",
      "State":     "CA",
      "Zip":       "94107",
      "Country":   "US"
    },
    {
      "precision": "zip",
      "Latitude":  37.371991,
      "Longitude": -122.026020,
      "Address":   "",
      "City":      "SUNNYVALE",
      "State":     "CA",
      "Zip":       "94085",
      "Country":   "US"
    }
  ])");

  std::cout << jsxxn::serialize(parsed) << std::endl;
  
  // switching json object with another json object
  parsed = jsxxn::parse(R"({
    "Image": {
        "Width":  800,
        "Height": 600,
        "Title":  "View from 15th Floor",
        "Thumbnail": {
            "Url":    "http://www.example.com/image/481989943",
            "Height": 125,
            "Width":  100
        },
        "Animated" : false,
        "IDs": [116, 943, 234, 38793]
      }
  })");

  std::cout << jsxxn::serialize(parsed) << std::endl;
    

  return 0;
}