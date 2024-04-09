#include "json_impl.h"
#include "json.h"

#include <string_view>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <cfloat>
#include <climits>

namespace json {

  struct LexState {
    std::string_view str;
    std::size_t curr;
    const std::size_t size;
    LexState(std::string_view str) : str(str), curr(0), size(str.length()) {}
  };

  std::string sec_string(std::string_view v, std::size_t start, std::size_t end);
  bool exact_match(std::string_view str, std::string_view check, std::size_t start);
  const char* ascii_cstr(char ch);

  inline std::string ascii_str(char ch) {
    return std::string(ascii_cstr(ch));
  }

  inline std::string sec_string(std::string_view v, std::size_t ind) {
    return sec_string(v, utf8beg(v, ind), utf8gnext(v, ind));
  }

  inline std::string_view interpret_utf8char(std::string_view utf8char) {
    if (utf8char.size() == 1UL && static_cast<std::uint8_t>(utf8char[0]) <= 0x7F)
      return ascii_cstr(utf8char[0]);
    return utf8char;
  }

  inline std::string utf8charstr(std::string_view utf8char) {
    return std::string(interpret_utf8char(utf8char));
  }


  Token tokenize_number(LexState& ls);
  Token tokenize_int(LexState& ls);
  Token tokenize_float(LexState& ls);
  Token tokenize_string(LexState& ls);
  void consume_comments(LexState& ls);
  Token consume_keyword(LexState& ls, std::string_view keyword, TokenLiteral matched_type, TokenType matched_token_type);

  std::vector<Token> tokenize(std::string_view str) {
    std::vector<Token> res;
    LexState ls(str);

    while (ls.curr < ls.size) {
      switch (ls.str[ls.curr]) {
        case '{': res.push_back(Token(TokenType::LEFT_BRACE, "{")); ls.curr++; break;
        case '}': res.push_back(Token(TokenType::RIGHT_BRACE, "}")); ls.curr++; break;
        case ',': res.push_back(Token(TokenType::COMMA, ",")); ls.curr++; break;
        case '[': res.push_back(Token(TokenType::LEFT_BRACKET, "[")); ls.curr++; break;
        case ']': res.push_back(Token(TokenType::RIGHT_BRACKET, "]")); ls.curr++; break;
        case ':': res.push_back(Token(TokenType::COLON, ":")); ls.curr++; break;
        case '"': res.push_back(tokenize_string(ls)); break;
        case '/': consume_comments(ls); break;
        case ' ':
        case '\r':
        case '\n':
        case '\t': ls.curr++; break;
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
        case '9': res.push_back(tokenize_number(ls)); break;
        case 't': res.push_back(consume_keyword(ls, "true", TokenLiteral(true), TokenType::TRUE)); break;
        case 'f': res.push_back(consume_keyword(ls, "false", TokenLiteral(false), TokenType::FALSE)); break;
        case 'n': res.push_back(consume_keyword(ls, "null", TokenLiteral(nullptr), TokenType::NULLPTR)); break;
        default: {
          std::stringstream errmsg;
          errmsg << "Unknown unhandled character: '" << interpret_utf8char(utf8gat(ls.str, ls.curr))
          << "' at index " << ls.curr << " " << sec_string(ls.str, ls.curr); 
          throw std::runtime_error(errmsg.str()); 
        }
      }
    }

    res.push_back(Token(TokenType::END_OF_FILE, nullptr));
    return res;
  }

  /**
   * Looks ahead from the current number token in order to classify a number as
   * either a float or an int, and then dispatches either toward tokenize_float
   * or tokenize_int respectively in order to parse the number.
   * 
   * Also handles many parsing errors, which largely simplifies the
   * implementation of tokenize_int and tokenize_float
   * 
   * Errors handled include:
   *  Missing integer part: (Ex: "-", ".", "-.", "-E13")
   *  Leading Zeros: (Ex: "012", "012.53")
   *  Invalid exponential part: (Ex: )
   *  Trailing Decimal Point: (Ex: "1.", "123.")
   * 
   * Therefore, these errors do not have to be checked for in either
   * tokenize_int or tokenize_float
  */
  Token tokenize_number(LexState& ls) {
    std::size_t lookahead = ls.curr;
    lookahead += ls.str[lookahead] == '-'; // consume -

    if ((stridx(ls.str, lookahead)) == '.')
      throw std::runtime_error("[tokenize_number] decimal with no integer part");

    if (!isdigit(stridx(ls.str, lookahead)))
      throw std::runtime_error("[tokenize_number] no integer part");

    if (ls.str[lookahead] == '0' && isdigit(stridx(ls.str, lookahead + 1)))
      throw std::runtime_error("[tokenize_number] leading zeros detected"); 

    for (; lookahead < ls.size && isdigit(ls.str[lookahead]); lookahead++); // consume integer part

    if (stridx(ls.str, lookahead) == '.') { // float handling
      if (!isdigit(stridx(ls.str, lookahead + 1)))
        throw std::runtime_error("[tokenize_number] trailing decimal point");
      return tokenize_float(ls);
    }

    if (stridx(ls.str, lookahead) == 'e' || stridx(ls.str, lookahead) == 'E') { // exponential part
      lookahead++;

      /**
       * I decided to not bother for missing integer parts on the exponentials here,
       * as tokenize_float would need to check anyway since there is an early
       * exit whenever a decimal point is detected. Therefore, both tokenize_int
       * and tokenize_float check for a missing integer part
      */
      switch (stridx(ls.str, lookahead)) {
        case '-': return tokenize_float(ls);
        case '+':
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': case '8':
        case '9': return tokenize_int(ls);
        case '\0': throw std::runtime_error("[tokenize_number] " // handle null from stridx
          "Exponential part followed by NUL");
        default: throw std::runtime_error("Exponential part followed by "
            "invalid character: " + utf8charstr(utf8gat(ls.str, lookahead)));
      }
    }

    // fall through to int, as no decimal point was found, nor was any
    // negative exponential part found
    return tokenize_int(ls);
  }

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
  Token tokenize_int(LexState& ls) {
    const std::size_t start = ls.curr; // 
    std::int64_t num = 0L;

    std::int64_t sign = 1 * (-1 * (ls.str[ls.curr] == '-'));
    ls.curr += ls.str[ls.curr] == '-';

    for (; ls.curr < ls.size && isdigit(ls.str[ls.curr]); ls.curr++) {
      std::int64_t digit = (ls.str[ls.curr] - '0');
      if ((INT64_MAX - digit) / 10LL <= num) {
        ls.curr = start;
        return tokenize_float(ls); 
      }

      num = num * 10LL + digit;
    }

    // note how we don't have to check for decimals here, as we assume that this
    // has already been confirmed by tokenize_number

    // read exponential part
    if (stridx(ls.str, ls.curr) == 'e' || stridx(ls.str, ls.curr) == 'E') {
      ls.curr++;
      constexpr int MAX_EXPONENTIAL = 20;
      unsigned int exponential = 0;
      ls.curr += stridx(ls.str, ls.curr) == '+';

      if (!isdigit(stridx(ls.str, ls.curr)))
        throw std::runtime_error("[tokenize_int] exponential missing integer part");

      for (; ls.curr < ls.size && isdigit(ls.str[ls.curr]); ls.curr++) {
        exponential = exponential * 10 + (ls.str[ls.curr] - '0');
        if (exponential > MAX_EXPONENTIAL) {
          ls.curr = start;
          return tokenize_float(ls); 
        }
      }

      for (; exponential != 0; exponential--) {
        if (INT64_MAX / 10LL <= num) {
          ls.curr = start;
          return tokenize_float(ls); 
        }

        num *= 10LL;
      }
    }

    num *= sign;
    return Token(TokenType::NUMBER, JSONNumber(num));
  }

  Token tokenize_float(LexState& ls) {
    double num = 0.0;
    int sign = 1.0 * (-1.0 * (ls.str[ls.curr] == '-'));
    ls.curr += ls.str[ls.curr] == '-'; 

    // read integer part
    for (; ls.curr < ls.size && isdigit(ls.str[ls.curr]); ls.curr++) {
      double digit = (ls.str[ls.curr] - '0');
      if ((DBL_MAX - digit) / 10.0 <= num) throw std::runtime_error("[tokenize_float] number too large");
      num = num * 10.0 + digit;
    }

    if (stridx(ls.str, ls.curr) == '.') { // read fractional part
      ls.curr++; // consume decimal
      double frac_mult = 0.1;
      for (; ls.curr < ls.size && isdigit(ls.str[ls.curr]); ls.curr++) {
        num += (ls.str[ls.curr] - '0') * frac_mult;
        frac_mult /= 10;
      }
    }

    // read exponential part
    if (stridx(ls.str, ls.curr) == 'e' || stridx(ls.str, ls.curr) == 'E') {
      ls.curr++;
      constexpr int MAX_EXPONENTIAL = 308;
      unsigned int exponential = 0;
      bool minus = stridx(ls.str, ls.curr) == '-';
      ls.curr += minus || stridx(ls.str, ls.curr) == '+';

      if (!isdigit(stridx(ls.str, ls.curr)))
        throw std::runtime_error("[tokenize_float] exponential missing integer part");

      for (; ls.curr < ls.size && isdigit(ls.str[ls.curr]); ls.curr++) {
        exponential = exponential * 10 + (ls.str[ls.curr] - '0');
      }

      if (exponential > MAX_EXPONENTIAL)
        throw std::runtime_error("[tokenize_float] number too large");

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

  Token tokenize_string(LexState& ls) {
    ls.curr++; // consume quotation
    const std::size_t start = ls.curr;
    bool closed = false;

    while (!closed && ls.curr < ls.size) {
      char ch = ls.str[ls.curr];
      switch (ch) {
        case '"': ls.curr++; closed = true; break; // consume final '"'
        // escape handling should be handed to the parser?
        case '\\': {
          char next = stridx(ls.str, ls.curr + 1);
          switch (next) {
            case '"': 
            case '\\': 
            case '/': 
            case 'b': 
            case 'f': 
            case 'n': 
            case 'r': 
            case 't': ls.curr += 2; break; 
            case 'u': { // unicode :(
              ls.curr += 2; // consume backslash and u

              if (ls.curr + 4 > ls.size)
                throw std::runtime_error("[json::tokenize_string] "
                "Incomplete unicode hex value");

              for (std::size_t i = 0; i < 4; i++) {
                if (!std::isxdigit(ls.str[ls.curr + i]))
                  throw std::runtime_error("[json::tokenize_string] Incomplete "
                  "unicode hex value");
              }
              
              ls.curr += 4;
            } break;
            case '\0':
              throw std::runtime_error("[json::tokenize_string] "
              "Unescaped backslash");
            default: {
              std::stringstream ss;
              ss << "[json::tokenize_string] Invalid escape sequence: \\" 
                << utf8charstr(utf8gat(ls.str, ls.curr + 1)) << " "
                << sec_string(ls.str, ls.curr + 1);

              throw std::runtime_error(ss.str());
            }
          }
        } break; // case '\\':
        case '\r':
        case '\n': throw std::runtime_error("[json::tokenize_string] Unclosed "
                          "String " + sec_string(ls.str, ls.curr));
        default: {
          /**
           * "All Unicode characters may be placed within the
           *  quotation marks, except for the characters that MUST be escaped:
           *  quotation mark, reverse solidus, and the control characters (U+0000
           *  through U+001F)." (RFC 8259 Section 7: Strings)
           *  So technically DEL is allowed? That had to be a mistake but whatever
          */
          if (std::iscntrl(ch) && ch != 127) 
            throw std::runtime_error("[json::tokenize_string] "
            " Unescaped control character inside of string: " +
            ascii_str(ch) + sec_string(ls.str, ls.curr));
            
          ls.curr++;
        }
      }
    }

    if (!closed)
      throw std::runtime_error("[json::tokenize_string] Unclosed String" + 
                                            sec_string(ls.str, ls.curr));
    return Token(TokenType::STRING, ls.str.substr(start, ls.curr - start - 1));
  }

  void consume_comments(LexState& ls) {
    switch (ls.curr + 1 < ls.size ? ls.str[ls.curr + 1] : '\0') {
      case '/': for (; ls.curr < ls.size && ls.str[ls.curr] != '\n'; ls.curr++); break;
      case '*': for (; ls.curr + 1 < ls.size; ls.curr++) {
        if (ls.str[ls.curr] == '*' && ls.str[ls.curr+1] == '/') {
          ls.curr += 2; break;
        }
      } break;
      default: throw std::runtime_error("[json::consume_comments] Unhandled slash");
    }
  }

  Token consume_keyword(LexState& ls, std::string_view keyword, TokenLiteral matched_type, TokenType matched_token_type) {
    if (exact_match(ls.str, keyword, ls.curr)) {
      ls.curr += keyword.length();
      return Token(matched_token_type, matched_type);
    }

    throw std::runtime_error("[consume_keyword] Tried to match " +
      std::string(keyword) + " at index" + std::to_string(ls.curr) +
      ". No match. ");
  }

  bool exact_match(std::string_view str, std::string_view check, std::size_t start) {
    std::size_t i = 0;
    for (; i < check.length() && i + start < str.length() && str[i + start] == check[i]; i++);
    return i == check.length();
  }

  std::string sec_string(std::string_view v, std::size_t start, std::size_t end) {
    std::stringstream ss;
    ss << "(" << linebef(v, start, 30) <<
      "->" << interpret_utf8char(v.substr(start, end - start)) << "<-" << linetoend(v, end, 30) << ")";
    return ss.str();
  }

  /* Used mainly for reporting char codes in errors which may have control characters */
  const char* ascii_cstr(char ch) {
    static const char* ascii_decs[256] = {"NUL","SOH","STX","ETX","EOT","ENQ",
      "ACK","\\a","\\b","\\t","\\n","\\v","\\f","\\r","SO","SI","DLE","DC1","DC2",
      "DC3","DC4","NAK","SYN","ETB","CAN","EM","SUB","ESC","FS","GS","RS","US",
      " ","!","\"","#","$","%","&","'","(",")","*","+",",","-",".","/","0","1",
      "2","3","4","5","6","7","8","9",":",";","<","=",">","?","@","A","B","C",
      "D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U",
      "V","W","X","Y","Z","[","\\","]","^","_","`","a","b","c","d","e","f","g",
      "h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y",
      "z","{","|","}","~","DEL", "128","129","130","131","132","133","134",
      "135","136","137","138","139","140","141","142","143","144","145","146",
      "147","148","149","150","151","152","153","154","155","156","157","158",
      "159","160","161","162","163","164","165","166","167","168","169","170",
      "171","172","173","174","175","176","177","178","179","180","181","182",
      "183","184","185","186","187","188","189","190","191","192","193","194",
      "195","196","197","198","199","200","201","202","203","204","205","206",
      "207","208","209","210","211","212","213","214","215","216","217","218",
      "219","220","221","222","223","224","225","226","227","228","229","230",
      "231","232","233","234","235","236","237","238","239","240","241","242",
      "243","244","245","246","247","248","249","250","251","252","253","254",
      "255"};
    return ascii_decs[static_cast<std::uint8_t>(ch)];
  }

};