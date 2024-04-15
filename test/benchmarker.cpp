
#include "jsxxn.h"
#include "jsxxn_impl.h"

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


constexpr std::chrono::nanoseconds BENCHMARK_ERR_TIME = std::chrono::nanoseconds(-1);

std::string ns_str(std::chrono::nanoseconds ns) {
  return ns == BENCHMARK_ERR_TIME ? "DNF" : (std::to_string(static_cast<double>(ns.count()) / 1E6) + "ms");
}

struct benchmark_data {
  std::chrono::nanoseconds tok_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds par_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds ser_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds repar_ser_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds strify_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds repar_strify_t = BENCHMARK_ERR_TIME;
  std::chrono::nanoseconds deq_t = BENCHMARK_ERR_TIME;
  unsigned int runs = 0;
};

#define TIMESTAMP(e) std::chrono::time_point e = std::chrono::high_resolution_clock::now()
#define SINCE(tp) (std::chrono::high_resolution_clock::now() - (tp))
#define DURATION(tp1, tp2) ((tp1) - (tp2))



benchmark_data benchmark(std::string_view json_str) {
  benchmark_data data;
  data.runs = 1;

  try {
    TIMESTAMP(before_tokenize);
    std::vector<json::Token> tokens = json::tokenize(json_str);
    data.tok_t = SINCE(before_tokenize);

    TIMESTAMP(before_parse);
    json::JSON parsed = json::parse(json_str);
    data.par_t = SINCE(before_parse);
    
    TIMESTAMP(before_serialize);
    std::string serialized = json::serialize(parsed);
    data.ser_t = SINCE(before_serialize);

    try {
      TIMESTAMP(before_reparse);
      json::JSON reparsed = json::parse(serialized);
      data.repar_ser_t = SINCE(before_reparse);

      TIMESTAMP(before_deep_equals);
      if (reparsed.equals_deep(parsed)) {
        data.deq_t = SINCE(before_deep_equals);
      }
    } catch (const std::runtime_error& err) { }

    TIMESTAMP(before_stringify);
    std::string stringified = json::stringify(parsed);
    data.strify_t = SINCE(before_stringify);

    try {
      TIMESTAMP(before_stringify_reparse);
      json::JSON stringify_reparse = json::parse(serialized);
      data.repar_strify_t = SINCE(before_stringify_reparse);
    } catch (const std::runtime_error& err) { }
    
    return data;
  } catch (const std::runtime_error& err) { return data; }

  return data;
}

benchmark_data benchmark(std::string_view json_str, unsigned int n_runs) {
  benchmark_data data;
  
  for (unsigned int i = 0; i < n_runs; i++) {
    benchmark_data run = benchmark(json_str);
    data.tok_t += run.tok_t;
    data.par_t += run.par_t;
    data.ser_t += run.ser_t;
    data.repar_ser_t += run.repar_ser_t;
    data.deq_t += run.deq_t;
    data.strify_t += run.strify_t;
    data.repar_strify_t += run.repar_strify_t;
    data.runs += run.runs;
  }

  data.tok_t /= n_runs;
  data.par_t /= n_runs;
  data.ser_t /= n_runs;
  data.repar_ser_t /= n_runs;
  data.deq_t /= n_runs;
  data.strify_t /= n_runs;
  data.repar_strify_t /= n_runs;
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

    benchmark_data data = benchmark(json_str, 30);

    std::cout << "Benchmarking " << id << "..." << std::endl;
    std::cout << "--------------------" << std::endl;
    std::cout << "Runs: " << data.runs << std::endl;
    std::cout << "Tokenizing Time: " << ns_str(data.tok_t) << std::endl;
    std::cout << "Full Parse Time: " << ns_str(data.par_t) << std::endl;
    std::cout << "Serialization Time: " << ns_str(data.ser_t) << std::endl;
    std::cout << "Full Reparse Time: " << ns_str(data.repar_ser_t) << std::endl;
    std::cout << "Deep Equality Time: " << ns_str(data.deq_t) << std::endl;
    std::cout << "Stringify Time: " << ns_str(data.strify_t) << std::endl;
    std::cout << "Stringify Full Reparse Time: " << ns_str(data.repar_strify_t) << std::endl;
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