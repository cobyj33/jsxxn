#include "json.h"

#include <stdexcept>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <string_view>
#include <variant>
#include <map>
#include <cfloat>
#include <stack>

#include <climits>
#include <cstddef>
#include <cstdint>

#include <iomanip>

#undef NULL
#undef EOF


namespace json {

  #define JSON_IMPL_MAX_NESTING_DEPTH 250

  enum class TokenType {
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    COMMA,
    COLON,

    TRUE,
    FALSE,
    NULL,

    NUMBER,
    STRING,

    EOF
  };

  struct Token {
    TokenType type;
    JSONLiteral val;
    Token(TokenType type, JSONLiteral val) : type(type), val(val) {}
  };

  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  std::string char_code_str(char ch);
  const char* json_token_type_cstr(TokenType tokenType);
  std::string json_token_type_str(TokenType tokenType);
  std::string json_token_str(Token token);
  bool exact_match(std::string_view str, std::string_view check, std::size_t start);
  char stridx(std::string_view str, std::size_t val);
  std::string u16_as_utf8(std::uint16_t val);
  std::string u16_as_hexstr(std::uint16_t val);
  std::uint16_t xdigit_as_u16(char ch);

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
            case 'n': res.push_back(this->consume_keyword("null", JSONLiteral(nullptr), TokenType::NULL)); break;
            default: throw std::runtime_error("Unknown unhandled character: '" + char_code_str(ch) + "'");
          }
        }

        res.push_back(Token(TokenType::EOF, nullptr));
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

        this->curr++; // consume ending quote
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

  std::string json_number_serialize(JSONNumber number) {
    return std::visit(overloaded {
      [&](const std::int64_t& num) { return std::to_string(num); },
      [&](const double& num) { return std::to_string(num); }
    }, number);
  }

  std::string json_literal_serialize(const JSONLiteral& literal) {
    return std::visit(overloaded {
      [&](const JSONNumber& number) { return json_number_serialize(number); },
      [&](const nullptr_t& nptr) {
        (void)nptr;
        return std::string("null");
      },
      [&](const bool& boolean) {
        return boolean ? std::string("true") : std::string("false");
      },
      [&](const std::string& str) {
        std::string serialized_str = "\"";

        for (std::string::size_type i = 0; i < str.size(); i++) {
          switch (str[i]) {
            case '"': serialized_str.append("\\\""); break;
            case '\\': serialized_str.append("\\\\"); break;
            case '\b': serialized_str.append("\\b"); break;
            case '\f': serialized_str.append("\\f"); break;
            case '\n': serialized_str.append("\\n"); break;
            case '\r': serialized_str.append("\\r"); break;
            case '\t': serialized_str.append("\\t"); break;
            default: {
              
              // control characters get turned into unicode escapes
              if (std::iscntrl(str[i])) {
                serialized_str += "\\u";
                serialized_str += u16_as_hexstr(str[i]);
              } else { // all other characters can just be unescaped
                serialized_str.push_back(str[i]);
              }

            }
          }
        }

        return serialized_str + "\"";
      }
    }, literal);
  }

  std::string serialize(const JSONValue& json, unsigned int depth) {
    if (depth > JSON_IMPL_MAX_NESTING_DEPTH) {
      throw std::runtime_error("[json::serialize] Exceeded max nesting "
      "depth of " + std::to_string(JSON_IMPL_MAX_NESTING_DEPTH));
    }

    return std::visit(overloaded { 
      [&](const JSONLiteral& literal) { return json_literal_serialize(literal);  },
      [&](const JSONObject& object) {
        if (object.size() == 0) return std::string("{}");
        std::string tab(depth * 2, ' ');
        std::string output = "{\n";

        JSONObject::size_type i = 0;
        for (const std::pair<const std::string, JSON>& entry : object) {
          output += tab;
          output += "  ";
          output += json_literal_serialize(entry.first); 
          output += ": "; 
          output += serialize(entry.second.value, depth + 1);
          if (i != object.size() - 1) output += ", ";
          output += "\n";

          i++;
        }

        output += tab;
        output += "}";
        return output;
      },
      [&](const JSONArray& arr) {
        if (arr.size() == 0) return std::string("[]");
        std::string tab(depth * 2, ' ');
        std::string output = "[\n";

        for (JSONArray::size_type i = 0; i < arr.size(); i++) {
          output += tab;
          output += "  ";
          output += serialize(arr[i].value, depth + 1);
          if (i != arr.size() - 1) output += ", ";
          output += "\n";
        }
        output += tab;
        output += "]";

        return output;
      }
    }, json);
  }

  std::string serialize(const JSONValue& json) {
    return serialize(json, 0);
  }

  class JSONParser {

    public:
      JSONParser() {}

      JSON parse(std::string_view str) {
        this->tokens = tokenize(str);

        this->curr = 0;
        JSON value = this->parse_value(0);
        if (this->curr <= this->tokens.size() &&
            this->tokens[this->curr].type != TokenType::EOF)
            throw std::runtime_error("[json::JSONParser::parse] Did not read "
              "all tokens as a value. ( Next Token: ( " +
              json_token_str(this->tokens[this->curr]) + " )");

        return value;
      }

    private:

      JSON parse_value(unsigned int depth) {
        if (depth > JSON_IMPL_MAX_NESTING_DEPTH)
          throw std::runtime_error("[json::JSONParser::parse_value] Exceeded "
          "max nesting depth of " + std::to_string(JSON_IMPL_MAX_NESTING_DEPTH));

        switch (this->tokens[this->curr].type) {
          case TokenType::LEFT_BRACE: return this->parse_object(depth);
          case TokenType::LEFT_BRACKET: return this->parse_array(depth);
          case TokenType::TRUE:
          case TokenType::FALSE:
          case TokenType::NULL:
          case TokenType::NUMBER:
          case TokenType::STRING: return JSON(this->tokens[this->curr++].val);

          case TokenType::EOF: throw std::runtime_error("[json::parse] expected value, got EOF");

          case TokenType::RIGHT_BRACE: 
          case TokenType::RIGHT_BRACKET:
          case TokenType::COLON:
          case TokenType::COMMA: // error
          default:
            throw std::runtime_error("[json::parse] expected a JSON value, "
            "got invalid token of type " +
            json_token_type_str(this->tokens[this->curr].type) +
            " ( " + json_token_str(this->tokens[this->curr] ) + " ) ");
        }
      }

      JSON parse_array(unsigned int depth) {
        // Array Grammar: "[" (value (, value)* )? "]"

        JSON arr(JSONValueType::ARRAY);
        this->curr++; //consume opening bracket

        if (this->tokens[this->curr].type == TokenType::RIGHT_BRACKET) {
          this->curr++;
          return arr;
        }

        // We know we must have a value inside of the array now.
        JSON value = this->parse_value(depth + 1);
        arr.push_back(value);

        while (this->tokens[this->curr].type != TokenType::RIGHT_BRACKET) {
          switch (this->tokens[this->curr].type) {
            case TokenType::EOF:
              throw std::runtime_error("[json::JSONParser::parse_array] Unclosed Array. Reached EOF");
            case TokenType::COMMA: {
              this->curr++;
              arr.push_back(this->parse_value(depth + 1));
            } break;
            default: throw std::runtime_error("[json::JSONParser::parse_array] "
              "Unexpected token hit, comma (\",\") or right bracket (\"]\") expected: " +
              json_token_str(this->tokens[this->curr]));
          }
        }

        this->curr++; // consume right bracket
        return arr;
      }


      // Grammar: STRING ":" value
      void parse_object_pair(JSON& obj, unsigned int depth) {
        if (this->tokens[this->curr].type != TokenType::STRING)
          throw std::runtime_error("[parse_object_pair] found object key " +
          json_token_str(this->tokens[this->curr]) + ". String expected.");

        std::string key = std::get<std::string>(this->tokens[this->curr].val);
        this->curr++;

        if (this->tokens[this->curr].type != TokenType::COLON)
          throw std::runtime_error("[parse_object_pair] expected colon, got " +
            json_token_type_str(this->tokens[this->curr].type) + 
            ". ( " + json_token_str(this->tokens[this->curr]) + " )");

        this->curr++;
        JSON value = this->parse_value(depth + 1);
        JSONObject& objval = std::get<JSONObject>(obj.value);
        objval.emplace(key, value);
      }

      JSON parse_object(unsigned int depth) {
        // Object Grammar: "{" ( ( STRING ":" value ) (, STRING ":"" value)* )? "}"
        JSON obj(JSONValueType::OBJECT);

        this->curr++;
        if (this->tokens[this->curr].type == TokenType::RIGHT_BRACE) {
          this->curr++;
          return obj;
        }

        this->parse_object_pair(obj, depth);

        while (this->tokens[this->curr].type != TokenType::RIGHT_BRACE) {
          switch (this->tokens[this->curr].type) {
            case TokenType::EOF:
              throw std::runtime_error("[json::parse_object] unclosed object");
            case TokenType::COMMA: {
             this->curr++;
             this->parse_object_pair(obj, depth);
            } break;
            default: throw std::runtime_error("[json::parse_object] "
              "Unexpected separator token of type " +
              json_token_type_str(this->tokens[this->curr].type) +
              ", expected colon (\"'\"). ( " + json_token_str(this->tokens[this->curr]) + " )");
          }
        }

        this->curr++; // consume right brace
        return obj;
      }

    private:
      std::vector<Token> tokens;
      std::size_t curr;
  };


  JSON parse(std::string_view str) {
    JSONParser parser;
    return parser.parse(str);
  }

  const char* json_value_type_str(JSONValueType jvt) {
    switch (jvt) {
      case JSONValueType::ARRAY: return "array";
      case JSONValueType::OBJECT: return "object";
      case JSONValueType::BOOLEAN: return "boolean";
      case JSONValueType::NUMBER: return "number";
      case JSONValueType::STRING: return "string";
      case JSONValueType::NULLPTR: return "null";
    }
    return "unknown";
  }

  JSONValueType json_literal_get_type(const JSONLiteral& literal) {
    return std::visit( overloaded {
      [&](const bool& val) { (void)val; return JSONValueType::BOOLEAN;  },
      [&](const nullptr_t& val) { (void)val; return JSONValueType::NULLPTR; },
      [&](const std::string& val) { (void)val; return JSONValueType::STRING; },
      [&](const JSONNumber& number) { (void)number; return JSONValueType::NUMBER; }
    }, literal);
  }

  JSONValueType json_value_get_type(const JSONValue& value) {
    return std::visit(overloaded { 
      [&](const JSONObject& obj) { (void)obj; return JSONValueType::OBJECT; },
      [&](const JSONArray& arr) { (void)arr; return JSONValueType::ARRAY; },
      [&](const JSONLiteral& literal) {
        return json_literal_get_type(literal);
      }
    }, value);
  }

  bool json_number_equals_deep(const JSONNumber& a, const JSONNumber& b) {
    constexpr double DOUBLE_EPSILON = 1E-6;

    return std::visit(overloaded {
      [&](const std::int64_t& a1, const std::int64_t& b1) { return a1 == b1; },
      [&](const std::int64_t& a1, const double& b1) {
        return std::abs(static_cast<double>(a1) - b1) < DOUBLE_EPSILON;
      },
      [&](const double& a1, const std::int64_t& b1) {
        return std::abs(a1 - static_cast<double>(b1)) < DOUBLE_EPSILON;
      },
      [&](const double& a1, const double& b1) {
        return std::abs(a1 - b1) < DOUBLE_EPSILON;
      }
    }, a, b); 
  }


  bool json_literal_equals_deep(const JSONLiteral& a, const JSONLiteral& b) {
    return std::visit(overloaded {
      [&](const JSONNumber& a1, const JSONNumber& b1) {
        return json_number_equals_deep(a1, b1);  
      },
      [&](const nullptr_t& a, const nullptr_t& b) {
        (void)a; (void)b;
        return true;
      },
      [&](const bool& a, const bool& b) {
        return a == b;
      },
      [&](const std::string& a, const std::string& b) {
        return a == b;
      },
      [&](const auto& a, const auto& b) {
        (void)a; (void)b;
        return false;
      }
    }, a, b);
  }

  bool json_value_equals_deep(const JSONValue& a, const JSONValue& b) {
    return std::visit(overloaded {
      [&](const JSONLiteral& literal1, const JSONLiteral& literal2) {
        return json_literal_equals_deep(literal1, literal2);
      },
      [&](const JSONObject& obj1, const JSONObject& obj2) {
        if (obj1.size() != obj2.size()) return false;

        for (const std::pair<std::string, JSON> entry : obj1) {
          if (obj2.count(entry.first) == 0) return false;
          if (!json_value_equals_deep(entry.second.value, obj2.at(entry.first).value))
            return false;
        }
        return true;
      },
      [&](const JSONArray& arr1, const JSONArray& arr2) {
        if (arr1.size() != arr2.size()) return false;
        for (JSONArray::size_type i = 0; i < arr1.size(); i++) {
          if (!json_value_equals_deep(arr1[i].value, arr2[i].value))
            return false;
        }
        return true;
      },
      [&](const auto& a1, const auto& a2) {
        (void)a1; (void)a2;
        return false;
      }
    }, a, b);
  }

  JSON::JSON() { this->value = nullptr; }
  JSON::JSON(nullptr_t value) { this->value = value; }
  JSON::JSON(bool value) { this->value = value; }
  JSON::JSON(std::int8_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int16_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int32_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int64_t value) { this->value = value; }

  JSON::JSON(double value) { this->value = value; }
  

  JSON::JSON(const char* value) { this->value = std::string(value); }
  JSON::JSON(std::string_view value) { this->value = std::string(value); }
  JSON::JSON(std::string value) { this->value = value; }
  JSON::JSON(const std::string& value) { this->value = value; }
  JSON::JSON(std::string&& value) { this->value = value; }

  JSON::JSON(JSONNumber value) { this->value = value; }
  
  JSON::JSON(JSONLiteral value) { this->value = value; }
  
  JSON::JSON(const JSON& value) { this->value = value.value; }

  JSON::JSON(const JSONArray& value) { this->value = value; }

  JSON::JSON(JSONArray&& value) { this->value = value; }

  JSON::JSON(const JSONObject& value) { this->value = value; }

  JSON::JSON(JSONObject&& value) { this->value = value; }

  JSON::JSON(JSONValueType type) {
    switch (type) {
      case JSONValueType::ARRAY: this->value = JSONArray(); break;
      case JSONValueType::OBJECT: this->value = JSONObject(); break;
      case JSONValueType::BOOLEAN: this->value = false; break;
      case JSONValueType::NUMBER: this->value = 0; break;
      case JSONValueType::STRING: this->value = std::string(); break;
      case JSONValueType::NULLPTR: this->value = nullptr; break;
    }
  }

  JSONValueType JSON::type() {
    return json_value_get_type(this->value);
  }

  bool JSON::equals_deep(const JSON& other) {
    return json_value_equals_deep(this->value, other.value);
  }

  JSON& JSON::operator=(const JSON& other) {
    this->value = other.value;
    return *this;
  }

  JSON::operator bool() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const bool* boolean = std::get_if<bool>(literal)) {
        return *boolean;
      }
    }
    throw std::runtime_error("[JSON::operator bool()]");

  }

  JSON::operator std::string() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const std::string* str = std::get_if<std::string>(literal)) {
        return *str;
      }
    }
    throw std::runtime_error("[JSON::operator std::string()]");
  }

  JSON::operator JSONValue() { return this->value; }
  JSON::operator JSONValue&() { return this->value; }
  
  JSON::operator double() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const JSONNumber* number = std::get_if<JSONNumber>(literal)) {
        return std::visit(overloaded {
          [&](const std::int64_t val) { return (double)(val); },
          [&](const double val) { return (val); },
        }, *number);
      }
    }
    throw std::runtime_error("[JSON::operator double()]");
  }

  JSON::operator std::int64_t() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const JSONNumber* number = std::get_if<JSONNumber>(literal)) {
        return std::visit(overloaded {
          [&](const std::int64_t val) { return val; },
          [&](const double val) { return (std::int64_t)val; },
        }, *number);
      }
    }
    throw std::runtime_error("[JSON::operator std::int64_t()]");
  }

  JSON::operator nullptr_t() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (std::holds_alternative<nullptr_t>(*literal)) {
        return nullptr;
      }
    }
    throw std::runtime_error("[JSON::operator nullptr_t()]");
  }

  JSON JSON::operator[](std::string_view key) {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value)) {
      if (map->count(key) == 1) {
        std::string key_str = std::string(key);
        return map->at(key_str);
      }
      throw std::runtime_error("[JSON::operator[]] key " + std::string(key) + " not found"); 
    }
    throw std::runtime_error("[JSON::operator[]] searching key on non-object type");
  }

  JSON JSON::operator[](const char* key) {
    return (*this)[std::string_view(key)];
  }
  
  JSON JSON::operator[](std::size_t idx) {
    if (const JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      if (idx >= arr->size()) {
        throw std::runtime_error("[JSON::operator[]] index out of range"); 
      }
      return arr->at(idx);
    }
    throw std::runtime_error("[JSON::operator[]] indexing non-array type"); 
  }

  void JSON::push_back(JSON&& json) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      arr->push_back(json);
      return;
    }
    throw std::runtime_error("[JSON::operator[]] pushing on non-array type"); 
  }

  void JSON::push_back(const JSON& json) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      arr->push_back(json);
      return;
    }
    throw std::runtime_error("[JSON::operator[]] pushing on non-array type"); 
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

  const char* json_token_type_cstr(TokenType tokenType) {
    switch (tokenType) {
      case TokenType::LEFT_BRACE: return "left brace";
      case TokenType::RIGHT_BRACE: return "right brace";
      case TokenType::LEFT_BRACKET: return "left bracket";
      case TokenType::RIGHT_BRACKET: return "right bracket";
      case TokenType::COMMA: return "comma";
      case TokenType::COLON: return "colon";
      case TokenType::TRUE: return "true";
      case TokenType::FALSE: return "false";
      case TokenType::NULL: return "null";
      case TokenType::NUMBER: return "number";
      case TokenType::STRING: return "string";
      case TokenType::EOF: return "eof";
    }
    return "unknown";
  }

  std::string json_token_type_str(TokenType tokenType) {
    return std::string(json_token_type_cstr(tokenType));
  }

  std::string json_token_str(Token token) {
    return "Token: { TokenType type: \"" + json_token_type_str(token.type) +
    "\", value: " + json_literal_serialize(token.val) + " }";
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

  std::string u16_as_hexstr(std::uint16_t val) {
    std::stringstream res;
    res << std::setfill('0') << std::setw(sizeof(std::uint16_t)*2);
    res << std::hex << val;
    return res.str();
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

  std::uint16_t xdigit_as_u16(char ch) {
    return (ch >= '0' && ch <= '9') * static_cast<std::uint16_t>(ch - '0') +
      (ch >= 'A' && ch <= 'F') * (static_cast<std::uint16_t>(ch - 'A' + 10)) +
      (ch >= 'a' && ch <= 'f') * (static_cast<std::uint16_t>(ch - 'a' + 10));
  }

};