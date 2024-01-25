
#include "json.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

std::string read_file_to_string(std::string path) {
  std::ifstream file(path);
  if (file.bad()) throw std::runtime_error("File " + path + " could not be opened.");

  std::string res;
  std::string line;
  while (std::getline(file, line)) res += line + '\n';
  return res;
}

void parse_json_file(std::string path) {
  std::string json_file = read_file_to_string(path);
  std::cout << "Parsing " << path << "..." << std::endl;
  std::cout << "Attempting to parse file contents:\n" << json_file << std::endl;
  
  try {
    json::JSON parsed = json::parse(json_file);
    std::cout << "Success in parsing " << path << std::endl;

    std::cout << "Reserialized version: " << json::serialize(parsed) << std::endl;
  } catch (const std::runtime_error& err) {
    std::cerr << "Failed to parse " << path << ": " << err.what() << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Enter at least one file to open and parse" << std::endl;
    return 1;
  }


  for (int i = 1; i < argc; i++) {
    parse_json_file(argv[i]);
    std::cout << std::endl;
  }

  return 0;
}