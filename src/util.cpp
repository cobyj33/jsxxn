#include "jsxxn_impl.h"

#include <stdexcept>
#include <cassert>

namespace jsxxn {
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
    return std::visit(overloaded {
      [](const bool val) { (void)val; return JSONValueType::BOOLEAN;  },
      [](const std::nullptr_t val) { (void)val; return JSONValueType::NULLPTR; },
      [](const std::string& val) { (void)val; return JSONValueType::STRING; },
      [](const JSONNumber& number) { (void)number; return JSONValueType::NUMBER; }
    }, literal);
  }

  JSONValueType json_value_get_type(const JSONValue& value) {
    return std::visit(overloaded { 
      [](const JSONObject& obj) { (void)obj; return JSONValueType::OBJECT; },
      [](const JSONArray& arr) { (void)arr; return JSONValueType::ARRAY; },
      [](const JSONLiteral& literal) {
        return json_literal_get_type(literal);
      }
    }, value);
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
      case TokenType::NULLPTR: return "null";
      case TokenType::NUMBER: return "number";
      case TokenType::STRING: return "string";
      case TokenType::END_OF_FILE: return "end of file";
    }
    return "unknown";
  }

  std::string json_token_type_str(TokenType tokenType) {
    return std::string(json_token_type_cstr(tokenType));
  }

  std::string json_string_resolve(std::string_view v) {
    std::string ret;
    const std::size_t vlen = v.length();

    // not sure if reserving is actually faster
    #if 0
    const std::uint32_t initres = nhpo2_u32(vlen); 
    ret.reserve(initres);
    #endif 
    
    std::size_t i = 0;
    while (i < vlen) {
      switch (v[i]) {
        case '\\': {
          assert(i + 1 < v.length());
          switch (v[i + 1]) {
            case '"': ret.push_back('"'); i += 2; break; 
            case '\\': ret.push_back('\\'); i += 2; break; 
            case '/': ret.push_back('/'); i += 2; break; 
            case 'b': ret.push_back('\b'); i += 2; break; 
            case 'f': ret.push_back('\f'); i += 2; break; 
            case 'n': ret.push_back('\n'); i += 2; break; 
            case 'r': ret.push_back('\r'); i += 2; break; 
            case 't': ret.push_back('\t'); i += 2; break; 
            case 'u': { // unicode :(
              i += 2; // consume \u
              assert(std::isxdigit(v[i]));
              std::uint16_t hexval = xdigit_as_u16(v[i++]);
              assert(std::isxdigit(v[i]));
              hexval = (hexval << 4) + xdigit_as_u16(v[i++]);
              assert(std::isxdigit(v[i]));
              hexval = (hexval << 4) + xdigit_as_u16(v[i++]);
              assert(std::isxdigit(v[i]));
              hexval = (hexval << 4) + xdigit_as_u16(v[i++]);
              ret += u16_as_utf8(hexval);
            } break;
            default: i += 2; break; // not even worried about invalid escapes here fr. 
          }
        } break;
        default: ret += v[i]; i++;
      }
    }

    return ret;
  }

  std::string json_token_literal_serialize(TokenLiteral literal) {
    // still not sure if std::visit or a if-chain is faster

    #if 0
    if (std::holds_alternative<JSONNumber>(literal)) {
      JSONNumber num = std::get<JSONNumber>(literal);
      return json_number_serialize(num);
    } else if (std::holds_alternative<std::string_view>(literal)) {
      std::string_view v = std::get<std::string_view>(literal);
      return std::string(v);
    } else if (std::holds_alternative<bool>(literal)) {
      bool b = std::get<bool>(literal);
      return std::to_string(b);
    }

    return std::string("null");
    #endif 

    return std::visit(overloaded {
      [](const JSONNumber number) {
        return json_number_serialize(number);
      },
      [](const std::nullptr_t& nptr) {
        (void)nptr;
        return std::string("null");
      },
      [](const bool boolean) {
        return boolean ? std::string("true") : std::string("false");
      },
      [](const std::string_view& str) {
        return std::string(str);
      }
    }, literal);
  }

  std::string json_token_str(Token token) {
    return "{ TokenType type: \"" + json_token_type_str(token.type) +
    "\", value: '" + json_token_literal_serialize(token.val) + "' }";
  }
};