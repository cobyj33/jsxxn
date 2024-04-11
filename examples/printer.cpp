#include "printer.h"

#include "port.h"
#include <iostream>
#include <fstream>



std::string read_file_to_string(std::string path);

int printer_main(int argc, char** argv, JSONPrinterFunc printer_func) {
  std::optional<bool> stdin_is_tty = json_is_stdin_atty();
  bool piped = false;
  if (stdin_is_tty.has_value()) piped = !(stdin_is_tty.value());
  if (argc < 2 && !piped) {
    std::cerr << "Enter a file or JSON String" << std::endl;
    return 1;
  }

  int exitcode = EXIT_FAILURE;

  if (piped) { // pipe or something else
    std::string json_str;
    std::string line;
    while (std::getline(std::cin, line)) json_str += line;

    try {
      json::JSON parsed = json::parse(json_str);
      std::cout << printer_func(parsed) << std::endl;
      exitcode = EXIT_SUCCESS;
    } catch (const std::runtime_error& err) {
      std::cerr << "Error while parsing piped input: " << err.what() << std::endl;
    }
  }


  for (int i = 1; i < argc; i++) {
    std::string json_str;
    bool is_json_file = false;
    try {
      json_str = read_file_to_string(argv[i]);
      is_json_file = true;
    } catch (const std::runtime_error& err) {
      json_str = std::string(argv[i]);
    }

    try {
      json::JSON parsed = json::parse(json_str);
      std::cout << printer_func(parsed) << std::endl;
      exitcode = EXIT_SUCCESS;
    } catch (const std::runtime_error& err) {
      if (is_json_file) {
        std::cerr << "Error while parsing json file '" << argv[i] << "':\n\t" <<
          err.what() << std::endl; 
      } else {
        std::cerr << "Error while parsing json input at argument #" << i << ":"
          << "\n\t" << err.what() << std::endl;
      }
    }
  }

  return exitcode;
}

std::string read_file_to_string(std::string path) {
  std::ifstream file(path);
  if (file.bad() || !file.is_open())
    throw std::runtime_error("File " + path + " could not be opened.");

  std::string res;
  std::string line;
  while (std::getline(file, line)) {
    res += line;
    res += '\n';
  }

  return res;
}