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

  std::string json_token_str(Token token) {
    return "Token: { TokenType type: \"" + json_token_type_str(token.type) +
    "\", value: " + json_literal_serialize(token.val) + " }";
  }
};