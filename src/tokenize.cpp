#include "json_impl.h"

#include <string_view>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <cfloat>
#include <climits>

namespace json {
  char stridx(std::string_view str, std::size_t val);
  bool exact_match(std::string_view str, std::string_view check, std::size_t start);
  std::uint16_t xdigit_as_u16(char ch);
  std::string u16_as_utf8(std::uint16_t val);
  std::string char_code_str(char ch);

  class JSONTokenizer {
    public: 
      JSONTokenizer() {}

      std::vector<Token> tokenize(std::string_view str) {
        std::vector<Token> res;
        this->src = str;
        this->curr = 0;

        while (this->curr < str.length()) {
          char ch = this->src[this->curr];
          switch (ch) {
            case '{': res.push_back(Token(TokenType::LEFT_BRACE, "{")); this->curr++; break;
            case '}': res.push_back(Token(TokenType::RIGHT_BRACE, "}")); this->curr++; break;
            case ',': res.push_back(Token(TokenType::COMMA, ",")); this->curr++; break;
            case '[': res.push_back(Token(TokenType::LEFT_BRACKET, "[")); this->curr++; break;
            case ']': res.push_back(Token(TokenType::RIGHT_BRACKET, "]")); this->curr++; break;
            case ':': res.push_back(Token(TokenType::COLON, ":")); this->curr++; break;
            case '"': res.push_back(this->tokenize_string()); break;
            case '/': this->consume_comments(); break;
            case ' ':
            case '\r':
            case '\n':
            case '\t': this->curr++; break;
            case '.': // this is just going to be caught in tokenize_number as a leading decimal
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': res.push_back(this->tokenize_number()); break;
            case 't': res.push_back(this->consume_keyword("true", JSONLiteral(true), TokenType::TRUE)); break;
            case 'f': res.push_back(this->consume_keyword("false", JSONLiteral(false), TokenType::FALSE)); break;
            case 'n': res.push_back(this->consume_keyword("null", JSONLiteral(nullptr), TokenType::NULLPTR)); break;
            default: throw std::runtime_error("Unknown unhandled character: '" + char_code_str(ch) + "'");
          }
        }

        res.push_back(Token(TokenType::END_OF_FILE, nullptr));
        return res;
      }

    private:

      /**
       * This function is only called from within tokenize_number whever
       * these conditions are met:
       * * The number does not have a fractional part (.DIGITS)
       * * The number does not have a negative exponential part ( "e-" DIGITS | "E-" DIGITS)
       * 
       * These two cases will be true whenever tokenize_int is called,
       * and therefore do not need to be checked again here.
       * 
       * Note that tokenize_int will defer to tokenize_float upon an overflow
       * being detected, as a double has a much larger maximum value than a
       * 64-bit signed integer
      */
      Token tokenize_int() {
        const std::size_t start = this->curr; // 
        std::int64_t num = 0;
        std::int64_t sign = 1;

        if (this->src[this->curr] == '-') { // read minus sign
          this->curr++;
          sign = -1;
        }

        for (; this->curr < this->src.length() && std::isdigit(this->src[this->curr]); this->curr++) {
          std::int64_t digit = (this->src[this->curr] - '0');
          if ((INT64_MAX - digit) / 10 <= num) {
            this->curr = start;
            return this->tokenize_float(); 
          }

          num = num * 10LL + digit;
        }

        // read exponential part
        if (stridx(this->src, this->curr) == 'e' || stridx(this->src, this->curr) == 'E') {
          this->curr++;
          constexpr int MAX_EXPONENTIAL = 20;
          unsigned int exponential = 0;

          if (stridx(this->src, this->curr) == '+') this->curr++;

          if (!std::isdigit(stridx(this->src, this->curr)))
            throw std::runtime_error("[tokenize_int] exponential missing integer part");

          for (; this->curr < this->src.length() && std::isdigit(this->src[this->curr]); this->curr++) {
            exponential = exponential * 10 + (this->src[this->curr] - '0');
            if (exponential > MAX_EXPONENTIAL) {
              this->curr = start;
              return this->tokenize_float(); 
            }
          }

          for (; exponential != 0; exponential--) {
            if (INT64_MAX / 10LL <= num) {
              this->curr = start;
              return this->tokenize_float(); 
            }

            num *= 10LL;
          }
        }

        num *= sign;
        return Token(TokenType::NUMBER, JSONNumber(num));
      }

      Token tokenize_float() {
        double num = 0;
        int sign = 1;

        if (this->src[this->curr] == '-') { // read minus sign
          this->curr++;
          sign = -1;
        }

        // read integer part
        for (; this->curr < this->src.length() && std::isdigit(this->src[this->curr]); this->curr++) {
          double digit = (this->src[this->curr] - '0');
          if ((DBL_MAX - digit) / 10 <= num) throw std::runtime_error("[tokenize_float] number too large");
          num = num * 10 + digit;
        }

        if (stridx(this->src, this->curr) == '.') { // read fractional part
          this->curr++; // consume decimal
          double frac_mult = 0.1;
          for (; this->curr < this->src.length() && std::isdigit(this->src[this->curr]); this->curr++) {
            num += (this->src[this->curr] - '0') * frac_mult;
            frac_mult /= 10;
          }
        }

        // read exponential part
        if (stridx(this->src, this->curr) == 'e' || stridx(this->src, this->curr) == 'E') {
          this->curr++;
          bool minus = false;
          constexpr int MAX_EXPONENTIAL = 308;
          unsigned int exponential = 0;

          switch (stridx(this->src, this->curr)) {
            case '-': this->curr++; minus = true; break;
            case '+': this->curr++; minus = false; break;
          }

          if (!std::isdigit(stridx(this->src, this->curr)))
            throw std::runtime_error("[tokenize_float] exponential missing integer part");

          for (; this->curr < this->src.length() && std::isdigit(this->src[this->curr]); this->curr++) {
            exponential = exponential * 10 + (this->src[this->curr] - '0');
            if (exponential > MAX_EXPONENTIAL)
              throw std::runtime_error("[tokenize_float] number too large");
          }

          if (minus) {
            for (; exponential != 0; exponential--) num *= 0.1;
          } else {
            for (; exponential != 0; exponential--) {
              if (DBL_MAX / 10 <= num)
                throw std::runtime_error("[tokenize_float] number too large");
              num *= 10;
            }
          }
        }

        num *= sign;
        return Token(TokenType::NUMBER, JSONNumber(num));
      }

      /**
       * looks ahead to determine if an int or a float shall be parsed
       * 
       * Also handles many parsing errors, which largely simplifies the
       * implementation of tokenize_int and tokenize_float
      */
      Token tokenize_number() {
        std::size_t lookahead = this->curr;

        if (this->src[lookahead] == '-') lookahead++;

        if ((stridx(this->src, lookahead)) == '.')
          throw std::runtime_error("[tokenize_number] decimal with no integer part");

        if (!std::isdigit(stridx(this->src, lookahead)))
          throw std::runtime_error("[tokenize_number] minus with no integer part");

        if (this->src[lookahead] == '0' && std::isdigit(stridx(this->src, lookahead + 1)))
          throw std::runtime_error("[tokenize_number] leading zeros detected"); 

        for (; lookahead < this->src.length() && std::isdigit(this->src[lookahead]); lookahead++);

        if (stridx(this->src, lookahead) == '.') {
          lookahead++;
          if (!std::isdigit(stridx(this->src, lookahead)))
            throw std::runtime_error("[tokenize_number] trailing decimal point");
          return this->tokenize_float();
        }

        if (stridx(this->src, lookahead) == 'e' || stridx(this->src, lookahead) == 'E') {
          lookahead++;

          /**
           * I decided to not bother for missing integer parts on the exponentials here,
           * as tokenize_float would need to check anyway since there is an early
           * exit whenever a decimal point is detected. Therefore, both tokenize_int
           * and tokenize_float check for a missing integer part
          */
          switch (stridx(this->src, lookahead)) {
            case '-': return this->tokenize_float();
            case '+':
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7': case '8':
            case '9': return this->tokenize_int();
            default: throw std::runtime_error("Exponential part followed by "
                "invalid character: " + char_code_str(stridx(this->src, lookahead)));
          }
        }

        // fall through to int, as no decimal point was found, nor was any
        // negative exponential part found
        return this->tokenize_int();
      }

      Token tokenize_string() {
        this->curr++; // consume quotation
        std::string extracted_str;
        bool closed = false;

        for (; !closed && this->curr < this->src.length(); this->curr++) {
          char ch = this->src[this->curr];
          switch (ch) {
            case '"': closed = true; break;
            case '\\': {
              char next = stridx(this->src, this->curr + 1);
              switch (stridx(this->src, this->curr + 1)) {
                case '"': extracted_str.push_back('"'); this->curr++; break; 
                case '\\': extracted_str.push_back('\\'); this->curr++; break; 
                case '/': extracted_str.push_back('/'); this->curr++; break; 
                case 'b': extracted_str.push_back('\b'); this->curr++; break; 
                case 'f': extracted_str.push_back('\f'); this->curr++; break; 
                case 'n': extracted_str.push_back('\n'); this->curr++; break; 
                case 'r': extracted_str.push_back('\r'); this->curr++; break; 
                case 't': extracted_str.push_back('\t'); this->curr++; break; 
                case 'u': { // unicode :(
                  this->curr++; // consume u
                  std::uint16_t hexval = 0;
                  
                  if (this->curr + 4 >= this->src.size())
                    throw std::runtime_error("Incomplete unicode hex value");

                  for (int i = 0; i < 4; i++) {
                    this->curr++;
                    char xch = this->src[this->curr];
                    if (!std::isxdigit((xch)))
                      throw std::runtime_error("Incomplete unicode hex value");
                    hexval = hexval * 16 + xdigit_as_u16(xch);
                  }


                  extracted_str += u16_as_utf8(hexval);

                  // TODO: Implement UTF-8 substitution
                } break;
                case '\0':
                  throw std::runtime_error("[json::JSONTokenizer::tokenize_string] "
                  "Unescaped backslash");
                default:
                  throw std::runtime_error("[json::JSONTokenizer::tokenize_string] "
                  "Invalid escape sequence: \\" + char_code_str(next));
              }
            } break;
            case '\r':
            case '\n': throw std::runtime_error("[json::JSONTokenizer::tokenize_string] Unclosed String");
            default: {
              /**
               * 
               * "All Unicode characters may be placed within the
               *  quotation marks, except for the characters that MUST be escaped:
               *  quotation mark, reverse solidus, and the control characters (U+0000
               *  through U+001F)." (RFC 8259 Section 7: Strings)
               *  So technically DEL is allowed? That had to be a mistake but whatever
              */
              if (std::iscntrl(ch) && ch != 127) 
                throw std::runtime_error("[json::JSONTokenizer::tokenize_string] "
                " Unescaped control character inside of string: " +
                char_code_str(ch));

              extracted_str.push_back(ch);
            }
          }
        }

        if (!closed) throw std::runtime_error("Unclosed String");
        return Token(TokenType::STRING, extracted_str);
      }

      void consume_comments() {
        switch (this->curr + 1 < this->src.length() ? this->src[this->curr + 1] : '\0') {
          case '/': for (; this->curr < this->src.length() && this->src[this->curr] != '\n'; this->curr++); break;
          case '*': for (; this->curr + 1 < this->src.length(); this->curr++) {
            if (this->src[this->curr] == '*' && this->src[this->curr+1] == '/') {
              this->curr += 2; break;
            }
          } break;
          default: throw std::runtime_error("[json::JSONTokenizer::consume_comments] Unhandled slash");
        }
      }

      Token consume_keyword(std::string_view keyword, JSONLiteral matched_type, TokenType matched_token_type) {
        if (exact_match(this->src, keyword, this->curr)) {
          this->curr += keyword.length();
          return Token(matched_token_type, matched_type);
        }

        throw std::runtime_error("[consume_match] Tried to match " +
          std::string(keyword) + " at index" + std::to_string(this->curr) +
          ". No match. ");
      }

      std::string_view src;
      std::size_t curr;
  };

  std::vector<Token> tokenize(std::string_view str) {
    return JSONTokenizer().tokenize(str);
  }

  char stridx(std::string_view str, std::size_t val) {
    return val < str.size() ? str[val] : '\0';
  }

  bool exact_match(std::string_view str, std::string_view check, std::size_t start) {
    if (str.length() - start < check.length()) return false;

    std::size_t i = 0;
    for (; i < check.length() && str[i + start] == check[i]; i++);
    return i == check.length();
  }

  std::uint16_t xdigit_as_u16(char ch) {
    return (ch >= '0' && ch <= '9') * static_cast<std::uint16_t>(ch - '0') +
      (ch >= 'A' && ch <= 'F') * (static_cast<std::uint16_t>(ch - 'A' + 10)) +
      (ch >= 'a' && ch <= 'f') * (static_cast<std::uint16_t>(ch - 'a' + 10));
  }

  std::string u16_as_utf8(std::uint16_t val) {
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

  /* Used mainly for reporting char codes in errors which may have control characters */
  std::string char_code_str(char ch) {
    switch (ch) {
      case 0: return "NUL";
      case 1: return "SOH";
      case 2: return "STX";
      case 3: return "ETX";
      case 4: return "EOT";
      case 5: return "ENQ";
      case 6: return "ACK";
      case 7: return "\\a";
      case 8: return "BS";
      case 9: return "\\t";
      case 10: return "\\n";
      case 11: return "VT";
      case 12: return "\\f";
      case 13: return "\\r";
      case 14: return "SO";
      case 15: return "SI";
      case 16: return "DLE";
      case 17: return "DC1";
      case 18: return "DC2";
      case 19: return "DC3";
      case 20: return "DC4";
      case 21: return "NAK";
      case 22: return "SYN";
      case 23: return "ETB";
      case 24: return "CAN";
      case 25: return "EM";
      case 26: return "SUB";
      case 27: return "ESC";
      case 28: return "FS";
      case 29: return "GS";
      case 30: return "RS";
      case 31: return "US";
      case 127: return "DEL";
      default: return std::string() + ch;
    }
  }

};