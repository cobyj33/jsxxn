#ifndef JSXXN_EXAMPLES_PORT_H
#define JSXXN_EXAMPLES_PORT_H

#include <optional>

#if defined(__linux__) || defined(unix) || defined(__unix) || defined(__unix__)\
    || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__sun)
  #include <unistd.h>
  inline std::optional<bool> json_isatty(int fd) {
    return isatty(fd);
  }

  inline std::optional<bool> json_is_stdin_atty() {
    return isatty(STDIN_FILENO);
  }
  
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || \
      defined(__NT__) || defined(__CYGWIN__)
  inline std::optional<bool> json_is_stdin_atty() {
    return std::nullopt;
  }
#else
  inline std::optional<bool> json_is_stdin_atty() {
    return std::nullopt;
  }
#endif


#endif