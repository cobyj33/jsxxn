#ifndef JSON_IMPL_H
#define JSON_IMPL_H

#include "json.h"
#define JSON_IMPL_MAX_NESTING_DEPTH 250

#include <string_view>
#include <cstddef>

namespace json {
  enum class TokenType {
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    COMMA,
    COLON,

    TRUE,
    FALSE,
    NULLPTR,

    NUMBER,
    STRING,

    END_OF_FILE
  };

  typedef std::variant<std::nullptr_t, std::string_view, JSONNumber, bool> TokenLiteral;

  inline char stridx(std::string_view str, std::size_t val) {
    return val < str.size() ? str[val] : '\0';
  }

  inline std::uint32_t nhpo2_u32(std::uint32_t n) {
    n--;
    n |= n >> 1;   // Divide by 2^k for consecutive doublings of k up to 32,
    n |= n >> 2;   // and then or the results.
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
  }

  inline std::uint16_t xdigit_as_u16(char ch) {
    return (ch >= '0' && ch <= '9') * static_cast<std::uint16_t>(ch - '0') +
      (ch >= 'A' && ch <= 'F') * (static_cast<std::uint16_t>(ch - 'A' + 10)) +
      (ch >= 'a' && ch <= 'f') * (static_cast<std::uint16_t>(ch - 'a' + 10));
  }

  inline std::string u16_as_utf8(std::uint16_t val) {
    std::string res;
    if (val < 0x0080) { // ascii
      res += static_cast<char>(val);
      return res;
    } else if (val < 0x0800) {
      res += static_cast<char>(0b110'00000 + ((val & 0b0000'0111'1100'0000) >> 6));
      res += static_cast<char>(0b10'000000 + (val & 0b0000'0000'0011'1111));
      return res;
    } else { // val <= 0xFFFF
      res += static_cast<char>(0b1110'0000 + ((val & 0b1111'0000'0000'0000) >> 12));
      res += static_cast<char>(0b10'000000 + ((val & 0b0000'1111'1100'0000) >> 6));
      res += static_cast<char>(0b10'000000 + ((val & 0b0000'0000'0011'1111)));
      return res;
    }
  }


  inline std::size_t st_addcl(std::size_t a, std::size_t b) {
    return (SIZE_MAX - b) < a ? SIZE_MAX : a + b;
  }

  inline std::size_t st_subcl(std::size_t a, std::size_t b) {
    return (b > a) ? 0 : a - b;
  }

  inline std::string_view sv_ar(std::string_view v, std::size_t ind, std::size_t bef, std::size_t af) {
    return v.substr(st_subcl(ind, bef), st_addcl(ind, af) - st_subcl(ind, bef));
  }

  inline std::string_view sv_ar(std::string_view v, std::size_t ind, std::size_t reach) {
    return v.substr(st_subcl(ind, reach), st_addcl(ind, reach) - st_subcl(ind, reach));
  }

  inline std::string str_ar(std::string_view v, std::size_t ind, std::size_t bef, std::size_t af) {
    return std::string(sv_ar(v, ind, bef, af));
  }

  inline std::string str_ar(std::string_view v, std::size_t ind, std::size_t reach) {
    return std::string(sv_ar(v, ind, reach));
  }

  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  struct Token {
    TokenType type;
    TokenLiteral val;
    Token(TokenType type, TokenLiteral val) : type(type), val(val) {}
  };

  std::vector<Token> tokenize(std::string_view str);

  // assumes a valid json string
  std::string json_string_resolve(std::string_view v);
  
  const char* json_token_type_cstr(TokenType tokenType);
  std::string json_token_type_str(TokenType tokenType);
  std::string json_token_str(Token token);
};


#endif