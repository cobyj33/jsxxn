#include "json_impl.h"

#include <stdexcept>

namespace json {
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
      [](const bool& val) { (void)val; return JSONValueType::BOOLEAN;  },
      [](const std::nullptr_t& val) { (void)val; return JSONValueType::NULLPTR; },
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
    const std::uint32_t initres = nhpo2_u32(vlen); 
    ret.reserve(initres);

    for (std::size_t i = 0; i < vlen; i++) {
      switch (v[i]) {
        case '\\': {
          char next = stridx(v, i + 1);
          switch (next) {
            case '"': ret.push_back('"'); i++; break; 
            case '\\': ret.push_back('\\'); i++; break; 
            case '/': ret.push_back('/'); i++; break; 
            case 'b': ret.push_back('\b'); i++; break; 
            case 'f': ret.push_back('\f'); i++; break; 
            case 'n': ret.push_back('\n'); i++; break; 
            case 'r': ret.push_back('\r'); i++; break; 
            case 't': ret.push_back('\t'); i++; break; 
            case 'u': { // unicode :(
              std::uint16_t hexval = 0;
              i++; // consume u
              hexval = (hexval << 4) + xdigit_as_u16(v[i++]);
              hexval = (hexval << 4) + xdigit_as_u16(v[i++]);
              hexval = (hexval << 4) + xdigit_as_u16(v[i++]);
              hexval = (hexval << 4) + xdigit_as_u16(v[i++]);
              ret += u16_as_utf8(hexval);
            } break;
            default: i++; break;
          }
        } break;
        default: ret += v[i];
      }
    }

    return ret;
  }

  std::string json_token_literal_serialize(TokenLiteral literal) {

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

    // return std::visit(overloaded {
    //   [](const JSONNumber& number) { return json_number_serialize(number); },
    //   [](const std::nullptr_t& nptr) {
    //     (void)nptr;
    //     return std::string("null");
    //   },
    //   [](const bool& boolean) {
    //     return boolean ? std::string("true") : std::string("false");
    //   },
    //   [](const std::string_view& str) {
    //     return std::string(str);
    //   }
    // }, literal);
  }

  std::string json_token_str(Token token) {
    return "{ TokenType type: \"" + json_token_type_str(token.type) +
    "\", value: '" + json_token_literal_serialize(token.val) + "' }";
  }
};