
#include "json.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include <filesystem>

/**
 * parser.cpp: PARser / SERializer
 * A utility to parse and serialize json files or json strings in order to view 
 * and report successes or failures in parsing and serialization.
*/

std::string read_file_to_string(std::string path) {
  std::ifstream file(path);
  if (file.bad() || !file.is_open())
    throw std::runtime_error("File " + path + " could not be opened.");

  std::string res;
  std::string line;
  while (std::getline(file, line)) res += line + '\n';
  return res;
}

void test_parsing(std::string id, std::string json_str) {
  std::cout << "--------------------" << std::endl;
  std::cout << "Parsing " << id << "..." << std::endl;
  std::cout << "Attempting to parse json contents:\n" << json_str << std::endl;


  try {  
    json::JSON parsed = json::parse(json_str);
    std::cout << "SUCCESS in parsing " << id << "." << std::endl;

    std::string serialized = json::serialize(parsed);
    std::cout << "Reserialized version of " << id << ":" << std::endl;
    std::cout << serialized << std::endl;

    try {
      json::JSON reparsed = json::parse(serialized);
      std::cout << "SUCCESS in reparsing serialized input." << std::endl;
    } catch (const std::runtime_error& err) {
      std::cerr << "FAILED to reparse serialized input." << std::endl;
    }
  } catch (const std::runtime_error& err) {
    std::cerr << "FAILED to parse " << id << ": " << err.what() << std::endl;
  }
  std::cout << "--------------------" << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Enter at least one file or json string to open and parse" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    try {
      std::string json_str = read_file_to_string(argv[i]);
      test_parsing(argv[i], json_str);
    } catch (const std::runtime_error& err) {
      test_parsing("CLI Argument " + std::to_string(i), argv[i]);
    }

    std::cout << std::endl;
  }

  return 0;
}