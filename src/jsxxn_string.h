#ifndef JSXXN_UTF_H
#define JSXXN_UTF_H

#include <string_view>
#include <string>
#include <cstddef>

namespace jsxxn {
  constexpr inline char stridx(std::string_view str, std::size_t val) {
    return val < str.length() ? str[val] : '\0';
  }

  constexpr inline char xdigit_as_ch(unsigned char ch) {
    switch (ch) {
      case 0: return '0';
      case 1: return '1';
      case 2: return '2';
      case 3: return '3';
      case 4: return '4';
      case 5: return '5';
      case 6: return '6';
      case 7: return '7';
      case 8: return '8';
      case 9: return '9';
      case 10: return 'A';
      case 11: return 'B';
      case 12: return 'C';
      case 13: return 'D';
      case 14: return 'E';
      case 15: return 'F';
    }
    return ch;
  }

  /**
   * Adds a to b, while clamping the result to never overflow past SIZE_MAX
  */
  constexpr inline std::size_t st_addcl(std::size_t a, std::size_t b) {
    return (SIZE_MAX - b < a) ? SIZE_MAX : (a + b);
  }

  /**
   * Subtract b from a ( do (a - b) ), while clamping the result to never
   * roll under 0.
  */
  constexpr inline std::size_t st_subcl(std::size_t a, std::size_t b) {
    return (b <= a) ? (a - b) : 0UL;
  }



  constexpr inline std::uint16_t xdigit_as_u16(char ch) {
    return (ch >= '0' && ch <= '9') * static_cast<std::uint16_t>(ch - '0') +
      (ch >= 'A' && ch <= 'F') * (static_cast<std::uint16_t>(ch - 'A' + 10)) +
      (ch >= 'a' && ch <= 'f') * (static_cast<std::uint16_t>(ch - 'a' + 10));
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
  constexpr inline bool isutf8gstart(unsigned char ch) {
    return (ch <= 0x7F) || ((ch & 0x80) && (ch & 0x40));
  }

  /**
   * Returns the start of the next grapheme in the string view starting from ind
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
   * 
   * Can return v.length() as the position of the next string, which would be
   * out of bounds
  */
  constexpr inline std::size_t utf8gnext(std::string_view v, std::size_t ind) {
    ind = std::min(ind, st_subcl(v.length(), 1));
    if (ind < v.length()) ind++; // guard for empty strings
    while (ind < v.length() && !isutf8gstart(v[ind])) ind++;
    return ind;
  }

  /**
   * Returns the beginning of the utf-8 grapheme in the string view at ind 
   * THE CALLER SHOULD GUARANTEE THAT v IS NON-EMPTY
  */
  constexpr inline std::size_t utf8beg(std::string_view v, std::size_t ind) {
    ind = std::min(ind, st_subcl(v.length(), 1));
    while (ind > 0 && !isutf8gstart(v[ind])) ind--;
    return ind;
  }

  /**
   * Returns a view of the utf-8 grapheme specified at ind
   * THE CALLER SHOULD GUARANTEE THAT v IS NON-EMPTY
  */
  constexpr inline std::string_view utf8gat(std::string_view v, std::size_t ind) {
    std::size_t beg = utf8beg(v, ind);
    return v.substr(beg, utf8gnext(v, ind) - beg);
  }

  /**
   * Converts a ucs4 code point to a utf-8 string
  */
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

  /**
   * Works on bytes, not utf-8 code points
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view sv_ar(std::string_view v, std::size_t ind, std::size_t bef, std::size_t af) {
    ind = std::min(ind, st_subcl(v.length(), 1));
    return v.substr(std::min(st_subcl(ind, bef), v.length()), st_addcl(ind, af) - st_subcl(ind, bef));
  }

  /**
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view sv_ar(std::string_view v, std::size_t ind, std::size_t reach) {
    ind = std::min(ind, st_subcl(v.length(), 1));
    return v.substr(st_subcl(ind, reach), st_addcl(ind, reach) - st_subcl(ind, reach));
  }

  /**
   * Works on bytes, not utf-8 code points
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view sv_bef(std::string_view v, std::size_t ind, std::size_t bef) {
    ind = std::min(ind, st_subcl(v.length(), 1));
    return v.substr(std::min(st_subcl(ind, bef), v.length()), bef);
  }

  /**
   * Works on bytes, not utf-8 code points
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view sv_af(std::string_view v, std::size_t ind, std::size_t af) {
    return v.substr(std::min(st_addcl(ind, 1), v.length()), af);
  }

  /**
   * Works on bytes, not utf-8 code points
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  inline std::string str_ar(std::string_view v, std::size_t ind, std::size_t bef, std::size_t af) {
    return std::string(sv_ar(v, ind, bef, af));
  }

  /**
   * Works on bytes, not utf-8 code points
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  inline std::string str_bef(std::string_view v, std::size_t ind, std::size_t bef) {
    return std::string(sv_bef(v, ind, bef));
  }

  /**
   * Works on bytes, not utf-8 code points
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  inline std::string str_af(std::string_view v, std::size_t ind, std::size_t af) {
    return std::string(sv_af(v, ind, af));
  }

  /**
   * Includes the character specified at ind.
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view linetobeg(std::string_view v, std::size_t ind) {
    if (v.empty()) return "";

    std::size_t begin = std::min(ind, v.length() - 1);
    while (begin > 0 && v[begin] != '\n') begin--;
    begin += v[begin] == '\n';
    return v.substr(begin, utf8gnext(v, ind) - begin);
  }
  
  /**
   * Includes the character specified at ind
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view linetobeg(std::string_view v, std::size_t ind, std::size_t lim) {
    if (v.empty()) return "";

    std::size_t begin = std::min(ind, v.length() - 1);
    while (v[begin] != '\n' && begin > 0 && lim > 0) {
      lim -= isutf8gstart(v[begin]);
      begin--;
    }

    begin += v[begin] == '\n'; 
    return v.substr(begin, utf8gnext(v, ind) - begin);
  }

  /**
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view linebef(std::string_view v, std::size_t ind) {
    std::string_view tobeg = linetobeg(v, ind);
    return tobeg.substr(0, st_subcl(tobeg.length(), 1));
  }

  /**
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view linebef(std::string_view v, std::size_t ind, std::size_t lim) {
    std::string_view tobeg = linetobeg(v, ind, lim);
    return tobeg.substr(0, st_subcl(tobeg.length(), 1));
  }

  /**
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view linetoend(std::string_view v, std::size_t ind) {
    std::size_t end = ind;
    while (end < v.length() && !(v[end] == '\r' || v[end] == '\n')) end++;
    return v.substr(std::min(ind, v.length()), end - ind);
  }

  /**
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view linetoend(std::string_view v, std::size_t ind, std::size_t lim) {
    std::size_t end = ind;
    while (end < v.length() && !(v[end] == '\r' || v[end] == '\n') && lim > 0) {
      lim -= isutf8gstart(v[end]);
      end++;
    }
    // "end" should only end at the beginning of a grapheme, the end of the
    // string view, a carriage return, or a new line 
    return v.substr(std::min(ind, v.length()), end - ind);
  }

  constexpr inline std::string_view lineaf(std::string_view v, std::size_t ind) {
    std::string_view lte = linetoend(v, ind);
    return lte.substr(std::min(1UL, lte.length()));
  }

  constexpr inline std::string_view lineaf(std::string_view v, std::size_t ind, std::size_t lim) {
    std::string_view lte = linetoend(v, ind, lim);
    return lte.substr(std::min(1UL, lte.length()));
  }

  /**
   * Retrieve the line specified at ind. 
   * 
   * indexes above v.length() are taken as the line
   * 
   * Can be called on empty string views and will not return an error or generate
   * undefined memory access. Can be called with out of bound indexes and will
   * not return an error or generate undefined memory access.
  */
  constexpr inline std::string_view lineof(std::string_view v, std::size_t ind) {
    if (v.empty()) return "";

    std::size_t begin = std::min(ind, st_subcl(v.length(), 1)); 
    std::size_t end = std::min(ind, st_subcl(v.length(), 1));
    while (begin > 0 && v[begin] != '\n') begin--;
    begin += v.length() > 0 && v[begin] == '\n';
    while (end < v.length() && !(v[end] == '\r' || v[end] == '\n')) end++;
    return v.substr(begin, end - begin);
  }

}

#endif