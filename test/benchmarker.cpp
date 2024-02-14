
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
 * and report speeds in parsing and serialization.
 * 
 * The utility for this file will be outputted under the directory "test" in
 * the build tree
 * 
 * testing data is provided in the test/data directory of the source tree 
 * under test/data/passing and test/data/failing.
 * 
 * A time for parsing, serialization, reparsing, and deep equality will be
 * provided to stdout
 * 
 * It would be useful to add a redirection of stdout and stderr into
 * a file such as "> testing_file.txt 2>&1" in order to look over the benchmark
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

std::string chrono_nanoseconds_str(std::chrono::nanoseconds ns) {
  return std::to_string(static_cast<double>(ns.count()) / 1000000) + "ms";
}

constexpr std::chrono::nanoseconds BENCHMARK_ERR_TIME = std::chrono::nanoseconds(-1);

struct benchmark_data {
  std::chrono::nanoseconds par_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds ser_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds repar_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds deq_t = BENCHMARK_ERR_TIME;
};


benchmark_data benchmark(const std::string& json_str) {
  benchmark_data data;

  try {
    std::chrono::time_point before_parse = std::chrono::steady_clock::now();
    json::JSON parsed = json::parse(json_str);
    data.par_t = std::chrono::steady_clock::now() - before_parse;
    
    std::chrono::time_point before_serialize = std::chrono::steady_clock::now();
    std::string serialized = json::serialize(parsed);
    data.ser_t = std::chrono::steady_clock::now() - before_serialize;

    try {
      std::chrono::time_point before_reparse = std::chrono::steady_clock::now();
      json::JSON reparsed = json::parse(serialized);
      data.repar_t = std::chrono::steady_clock::now() - before_reparse;

      std::chrono::time_point before_deep_equals = std::chrono::steady_clock::now();
      if (reparsed.equals_deep(parsed)) {
        data.deq_t = std::chrono::steady_clock::now() - before_deep_equals;
      }
    } catch (const std::runtime_error& err) { return data; }
  } catch (const std::runtime_error& err) { return data; }

  return data;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Enter at least one file or json string to open and benchmark" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    std::string id;
    std::string json_str;

    try {
      json_str = read_file_to_string(argv[i]);
      id = std::string(argv[i]);
    } catch (const std::runtime_error& err) {
      id = "CLI Argument " + std::to_string(i);
      json_str = std::string(argv[i]);
    }

    benchmark_data data = benchmark(json_str);

    std::cout << "Benchmarking " << id << "..." << std::endl;
    std::cout << "--------------------" << std::endl;
    std::cout << "Parsing Time: " << chrono_nanoseconds_str(data.par_t) << std::endl;
    std::cout << "Serialization Time: " << chrono_nanoseconds_str(data.ser_t) << std::endl;
    std::cout << "Reparse Time: " << chrono_nanoseconds_str(data.repar_t) << std::endl;
    std::cout << "Deep Equality Time: " << chrono_nanoseconds_str(data.deq_t) << std::endl;
    std::cout << "--------------------" << std::endl;
    std::cout << std::endl;
  }

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