
#include "json.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include <filesystem>
#include <chrono>

/**
 * tester.cpp: 
 * A utility to parse and serialize json files or json strings in order to view 
 * and report successes or failures in parsing and serialization.
 * 
 * The utility for this file will be outputted under the directory "test" in
 * the build tree
 * 
 * testing data is provided in the test/data directory of the source tree 
 * under test/data/passing and test/data/failing.
 * 
 * A final count of all passed and failed tests will be produced at the
 * end of stdout
 * 
 * It would be useful to add a redirection of stdout and stderr into
 * a file such as "> testing_file.txt 2>&1" in order to look over the testing
 * data at once
 * 
 * Note that this utility can be used with any json file, and is not restricted
 * to only testing the parsing of files in the test/data directory. The test/data
 * directory is only meant as an ease of use for testing the parser.
*/

// test can be called for all files within these directories with globs toward
// test/data/passing/**/*.json and test/data/failing/**/*.json from your
// current working directory

std::string read_file_to_string(std::string path);

struct json_test_data {
  std::string id = "";
  std::string json_str = "";
  
  json_test_data() {}
  json_test_data(std::string id, std::string json_str) : id(id), json_str(json_str) {}
};

struct json_test_err {
  json_test_data data;
  std::string reason = "";
  json_test_err() {}
  json_test_err(json_test_data data, std::string reason) : data(data), reason(reason) {}
};


#define TIMESTAMP(e) std::chrono::time_point e = std::chrono::steady_clock::now()
#define SINCE(tp) (std::chrono::steady_clock::now() - (tp))
#define DURATION(tp1, tp2) ((tp1) - (tp2))

std::string ns_str(std::chrono::nanoseconds ns) {
  return std::to_string(static_cast<double>(ns.count()) / 1000000) + "ms";
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Enter at least one file or json string to open and test" << std::endl;
    return 1;
  }

  std::vector<json_test_data> passes;
  std::vector<json_test_err> failures;

  for (int i = 1; i < argc; i++) {
    json_test_data test;

    try {
      test.json_str = read_file_to_string(argv[i]);
      test.id = std::string(argv[i]);
    } catch (const std::runtime_error& err) {
      test.id = "CLI Argument " + std::to_string(i);
      test.json_str = std::string(argv[i]);
    }

    std::cout << "--------------------" << std::endl;
    std::cout << "Parsing " << test.id << "..." << std::endl;
    std::cout << "Attempting to parse json contents:\n" << test.json_str << std::endl;

    try {  
      TIMESTAMP(before_parse);
      json::JSON parsed = json::parse(test.json_str);
      TIMESTAMP(after_parse);
      std::cout << "SUCCESS in parsing " << test.id << "." << std::endl;
      std::cout << "Parsing Time: " << ns_str(DURATION(after_parse, before_parse)) << std::endl;
      
      TIMESTAMP(before_serialize);
      std::string serialized = json::serialize(parsed);
      TIMESTAMP(after_serialize);

      std::cout << "Reserialized version of " << test.id << ":" << std::endl;
      std::cout << serialized << std::endl;
      std::cout << "Serialization Time: " << ns_str(DURATION(after_serialize, before_serialize)) << std::endl;

      try {
        json::JSON reparsed = json::parse(serialized);
        std::cout << "SUCCESS in reparsing serialized input." << std::endl;

        TIMESTAMP(before_deep_equals);
        if (reparsed.equals_deep(parsed)) {
          TIMESTAMP(after_deep_equals);
          std::cout << "SUCCESS: Reparsed object equals parsed object" << std::endl;
          std::cout << "Deep Equality Time: " << ns_str(DURATION(after_deep_equals, before_deep_equals)) << std::endl;

          passes.push_back(test);
        } else {
          std::cerr << "FAILED: Reparsed input not detected as equal "
          "to parsed input" << std::endl;
          failures.push_back(json_test_err(test, "Reparsed and parsed input detected as unequal"));
        }
        
      } catch (const std::runtime_error& err) {
        std::cerr << "FAILED to reparse serialized input: " << err.what() << std::endl;
        failures.push_back(json_test_err(test, err.what()));
      }
    } catch (const std::runtime_error& err) {
      std::cerr << "FAILED to parse " << test.id << ": " << err.what() << std::endl;
      failures.push_back(json_test_err(test, err.what()));
    }

    std::cout << "--------------------" << std::endl;
    std::cout << std::endl;
  }

  std::cout << "Total: " << passes.size() + failures.size() << std::endl;
  std::cout << "Passed: " << passes.size() << std::endl;
  std::cout << "Failed: " << failures.size() << std::endl;

  return 0;
}

std::string read_file_to_string(std::string path) {
  std::ifstream file(path);
  if (file.bad() || !file.is_open())
    throw std::runtime_error("File " + path + " could not be opened.");

  std::string res;
  std::string line;
  while (std::getline(file, line)) res += line + '\n';
  return res;
}