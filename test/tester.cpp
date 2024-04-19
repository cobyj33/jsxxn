
#include "jsxxn.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include <filesystem>
#include <chrono>
#include <algorithm>

// Quick Start:
// Example command once built (all from build directory):
// Run Passing tests:
//  ./test/tester ../test/data/passing/*.json ../test/data/passing/**/*.json
// Run Failing tests:
//  ./test/tester ../test/data/failing/*.json ../test/data/failing/**/*.json
// Run tests and redirect output to a file:
//  ./test/tester <files> 2>&1 > file.txt
//  Ex: ./test/tester ../test/data/passing/*.json ../test/data/passing/**/*.json 2>&1 > my_file_name.txt

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
};

struct json_test_res {
  json_test_data tdata;
  bool success;
  jsxxn::JSON dom;
  std::string prettified;
  std::vector<std::string> msgs;
};

std::ostream& operator<<(std::ostream& os, json_test_res& tres) {
  os << "Test '" << tres.tdata.id << "': \n"
    << "Success: " <<  std::to_string(tres.success) << "\n"
    << "Inputted JSON String: \n\n" << tres.tdata.json_str << "\n";

  os << "Messages:\n";
  for (const std::string& msg : tres.msgs) {
    os << "  " << msg << "\n";
  }

  if (tres.success) {
    os << "\nPrettified Structure: " << "\n\n"
      << tres.prettified << "\n";
  }

  return os;
}

struct json_tests {
  std::vector<json_test_res> successes;
  std::vector<json_test_res> failures;
};

#define TIMESTAMP(e) std::chrono::time_point e = std::chrono::steady_clock::now()
#define SINCE(tp) (std::chrono::steady_clock::now() - (tp))
#define DURATION(tp1, tp2) ((tp1) - (tp2))

std::string ns_str(std::chrono::nanoseconds ns) {
  return std::to_string(static_cast<double>(ns.count()) / 1000000) + "ms";
}

bool test_serialize(const jsxxn::JSON& json, std::string inputname, std::string funcname, jsxxn::JSONSerializeFunc serfunc, json_test_res& test, std::string& res_str) {
  try {
    TIMESTAMP(before);
    res_str = serfunc(json);
    TIMESTAMP(after);
    test.msgs.push_back("SUCCESS in serializing " + inputname + " with " + funcname + " function");
    test.msgs.push_back("Serialization Time: " +  ns_str(DURATION(after, before)));
    return true;
  } catch (const std::runtime_error& err) {
    test.msgs.push_back("FAILED to serialize " + inputname + " with " + funcname + " function: " + std::string(err.what()));
    return false;
  }
}

bool test_parsing(std::string_view str, std::string inputname, json_test_res& test, jsxxn::JSON& res_json) {
  try {
    TIMESTAMP(before);
    res_json = jsxxn::parse(str);
    TIMESTAMP(after);
    test.msgs.push_back("SUCCESS in parsing " + inputname);
    test.msgs.push_back("Parsing Time: " +  ns_str(DURATION(after, before)));
    return true;
  } catch (const std::runtime_error& err) {
    test.msgs.push_back("FAILED to reparse " + inputname + ": " + std::string(err.what()));
    return false;
  }
}

bool test_self_equality(const jsxxn::JSON& a, std::string aname, json_test_res& res) {
  TIMESTAMP(before);
  if (a.equals_deep(a)) {
    TIMESTAMP(after);
    res.msgs.push_back("SUCCESS: " + aname + " equals itself");
    res.msgs.push_back("Deep Equality Time: " + ns_str(DURATION(after, before)));
    return true;
  } else {
    res.msgs.push_back("FAILED: " + aname + " detected as not equal "
    "to itself");
    return false;
  }
}

bool test_equality(const jsxxn::JSON& a, std::string aname, const jsxxn::JSON& b, std::string bname, json_test_res& res) {
  TIMESTAMP(before);
  if (a.equals_deep(b)) {
    TIMESTAMP(after);
    res.msgs.push_back("SUCCESS: " + aname + " equals " + bname);
    res.msgs.push_back("Deep Equality Time: " + ns_str(DURATION(after, before)));
    return true;
  } else {
    res.msgs.push_back("FAILED: " + aname + " detected as not equal "
    "to " + bname);
    return false;
  }
}

json_test_res run_test(const json_test_data& test) {
  json_test_res res;
  res.success = false;
  res.tdata = test;

  // Initial Parsing
  jsxxn::JSON parsed;
  if (!test_parsing(test.json_str, "Input", res, parsed)) return res;

  // Reserialization
  std::string prettified;
  std::string stringified;
  if (!test_serialize(parsed, "Original parsed input", "prettify", jsxxn::prettify, res, prettified)) return res;
  if (!test_serialize(parsed, "Original parsed input", "stringify", jsxxn::stringify, res, stringified)) return res;

  // Redeserialization
  jsxxn::JSON pretty_parsed;
  jsxxn::JSON stringy_parsed;
  if (!test_parsing(prettified, "Prettified Input", res, pretty_parsed)) return res;
  if (!test_parsing(stringified, "Stringified Input", res, stringy_parsed)) return res;

  // reflexive equals test
  if (!test_self_equality(parsed, "Originally Parsed Input", res)) return res;
  if (!test_self_equality(pretty_parsed, "Reparsed Prettified Input", res)) return res;
  if (!test_self_equality(stringy_parsed, "Reparsed Stringified Input", res)) return res;

  //transitive test
  if (!test_equality(parsed, "Originally Parsed Input", pretty_parsed, "Reparsed Prettified Input", res)) return res;
  if (!test_equality(pretty_parsed, "Reparsed Prettified Input", stringy_parsed, "Reparsed Stringified Input", res)) return res;
  if (!test_equality(stringy_parsed, "Reparsed Stringified Input", parsed, "Originally Parsed Input", res)) return res;

  res.success = true;
  res.dom = std::move(parsed);
  res.prettified = std::move(prettified);
  return res;
}


int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Enter at least one file or json string to open and test" << std::endl;
    return 1;
  }

  json_tests tests;

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

    json_test_res test_res = run_test(test);
    std::cout << test_res << std::endl;

    if (test_res.success) {
      tests.successes.push_back(test_res);
    } else {
      tests.failures.push_back(test_res);
    }

    std::cout << "--------------------" << std::endl;
    std::cout << std::endl;
  }

  std::cout << "Total: " << tests.successes.size() + tests.failures.size() << std::endl;
  std::cout << "Passed: " << tests.successes.size() << std::endl;
  std::cout << "Failed: " << tests.failures.size() << std::endl;

  return 0;
}

std::string read_file_to_string(std::string path) {
  std::ifstream file(path);
  if (file.bad() || !file.is_open())
    throw std::runtime_error("File " + path + " could not be opened.");

  std::string res;
  std::string line;
  while (std::getline(file, line)) {
    res += line;
    res.push_back('\n');
  } 

  return res;
}