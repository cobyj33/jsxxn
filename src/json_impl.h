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

  inline bool isdigit(char ch) {
    return ch >= '0' && (ch) <= '9';
  }

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

  inline std::string_view sv_bef(std::string_view v, std::size_t ind, std::size_t bef) {
    return v.substr(st_subcl(ind, bef), bef);
  }

  inline std::string_view sv_af(std::string_view v, std::size_t ind, std::size_t af) {
    return v.substr(st_addcl(ind, 1), af);
  }

  inline std::string str_ar(std::string_view v, std::size_t ind, std::size_t bef, std::size_t af) {
    return std::string(sv_ar(v, ind, bef, af));
  }

  inline std::string str_bef(std::string_view v, std::size_t ind, std::size_t bef) {
    return std::string(sv_bef(v, ind, bef));
  }

  inline std::string str_af(std::string_view v, std::size_t ind, std::size_t af) {
    return std::string(sv_af(v, ind, af));
  }

  inline std::string_view linetobeg(std::string_view v, std::size_t ind) {
    std::size_t begin = ind; 
    while (v[begin] != '\n' && begin > 0) begin--;
    begin += v[begin] == '\n';
    return v.substr(begin, (ind + 1) - begin);
  }

  inline std::string_view linetobeg(std::string_view v, std::size_t ind, std::size_t lim) {
    std::size_t begin = ind;
    while (v[begin] != '\n' && begin > 0 && lim > 0) {
      begin--;
      lim--;
    }
    begin += v[begin] == '\n';
    return v.substr(begin, (ind + 1) - begin);
  }

  inline std::string_view linebef(std::string_view v, std::size_t ind) {
    std::string_view tobeg = linetobeg(v, ind);
    return tobeg.substr(0, st_subcl(tobeg.length(), 1));
  }

  inline std::string_view linebef(std::string_view v, std::size_t ind, std::size_t lim) {
    std::string_view tobeg = linetobeg(v, ind, lim);
    return tobeg.substr(0, st_subcl(tobeg.length(), 1));
  }

  inline std::string_view linetoend(std::string_view v, std::size_t ind) {
    std::size_t end = ind + 1;
    while (!(v[end] == '\r' || v[end] == '\n') && end < v.length()) end++;
    return v.substr(ind, end - ind);
  }

  inline std::string_view linetoend(std::string_view v, std::size_t ind, std::size_t lim) {
    std::size_t end = ind;
    while (!(v[end] == '\r' || v[end] == '\n') && end < v.length() && lim > 0) {
      end++;
      lim--; 
    }
    return v.substr(ind, end - ind);
  }

  inline std::string_view lineaf(std::string_view v, std::size_t ind) {
    return linetoend(v, ind).substr(1);
  }

  inline std::string_view lineaf(std::string_view v, std::size_t ind, std::size_t lim) {
    return linetoend(v, ind, lim).substr(1);
  }

  inline std::string_view lineof(std::string_view v, std::size_t ind) {
    std::size_t begin = ind; 
    std::size_t end = ind;
    while (v[begin] != '\n' && begin > 0) begin--;
    begin += v[begin] == '\n';
    while (!(v[end] == '\r' || v[end] == '\n') && end < v.length()) end++;
    return v.substr(begin, end - begin);
  }

  /**
   * From the UTF-8 wikipedia page: https://en.wikipedia.org/wiki/UTF-8
   * Table detailing how different code point ranges are encoded in UTF-8 
   * +------------------+-----------------+----------+----------+----------+----------+
   * | First code point | Last code point |  Byte 1  |  Byte 2  |  Byte 3  |  Byte 4  |
   * +------------------+-----------------+----------+----------+----------+----------+
   * | U+0000           |      U+007F     | 0xxxxxxx |          |          |          |
   * | U+0080           |      U+07FF     | 110xxxxx | 10xxxxxx |          |          |
   * | U+0800           |      U+FFFF     | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
   * | U+010000         |     U+10FFFF    | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |
   * +------------------+-----------------+----------+----------+----------+----------+
  */

  /**
   * isutf8gstart: is utf8 grapheme start
  */
  inline bool isutf8gstart(unsigned char ch) {
    return (ch <= 0x7F) || ((ch & 0x80) && (ch & 0x40));
  }

  inline std::size_t utf8gnext(std::string_view v, std::size_t ind) {
    ind++;
    while (!isutf8gstart(v[ind])) ind++;
    return ind;
  }

  inline std::size_t utf8beg(std::string_view v, std::size_t ind) {
    while (!isutf8gstart(v[ind]) && ind > 0) ind--;
    return ind;
  }

  inline std::string_view utf8gat(std::string_view v, std::size_t ind) {
    std::size_t beg = utf8beg(v, ind);
    return v.substr(beg, utf8gnext(v, ind) - beg);
  }
  
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  struct Token {
    TokenType type;
    TokenLiteral val;
    Token(TokenType type, TokenLiteral val) : type(type), val(val) {}
    Token(const Token& token) : type(token.type), val(token.val) {}
    Token(Token&& token) : type(token.type), val(token.val) {}
  };

  struct LexState {
    std::string_view str;
    std::size_t curr;
    const std::size_t size;
    LexState(std::string_view str) : str(str), curr(0), size(str.length()) {}
  };


  std::vector<Token> tokenize(std::string_view str);

  Token nextToken(std::string_view str, std::size_t val);

  // assumes a valid json string
  std::string json_string_resolve(std::string_view v);
  
  const char* json_token_type_cstr(TokenType tokenType);
  std::string json_token_type_str(TokenType tokenType);
  std::string json_token_str(Token token);
};


#endif