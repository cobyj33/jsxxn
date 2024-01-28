#include "json.h"

#include <iostream>
#include <fstream>

// A simple pretty printer which takes in a path to a json file or a json string

std::string read_file_to_string(std::string path);

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Enter a file or JSON String to pretty print" << std::endl;
    return 1;
  }
  
  std::string json_str;
  try {
    json_str = read_file_to_string(argv[1]);
  } catch (const std::runtime_error& err) {
    json_str = std::string(argv[1]);
  }

  try {
    json::JSON parsed = json::parse(json_str);
    std::cout << json::serialize(parsed) << std::endl;
    return 0;
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    return 1;
  }
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