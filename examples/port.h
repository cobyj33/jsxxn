#ifndef JSXXN_EXAMPLES_PORT_H
#define JSXXN_EXAMPLES_PORT_H

#include <optional>

#if defined(__linux__) || defined(unix) || defined(__unix) || defined(__unix__)\
    || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__sun)
  #include <unistd.h>
  inline std::optional<bool> jsxxn_isatty(int fd) {
    return std::optional<bool>(isatty(fd));
  }

  /**
   * jsxxn_is_stdin_atty is used to decided if piped input is present or if 
   * terminal input is given. std::nullopt means that there was no way to 
   * determine if stdin does or does not exist as a tty connection, and 
   * basically just exists for unsupported platforms. On Unix-like platforms 
   * though it should always work.
  */
  inline std::optional<bool> jsxxn_is_stdin_atty() {
    return std::optional<bool>(isatty(STDIN_FILENO));
  }
  
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || \
      defined(__NT__) || defined(__CYGWIN__)
  inline std::optional<bool> jsxxn_is_stdin_atty() {
    return std::nullopt;
  }
#else
  inline std::optional<bool> jsxxn_is_stdin_atty() {
    return std::nullopt;
  }
#endif


#endif