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

  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  struct Token {
    TokenType type;
    JSONLiteral val;
    Token(TokenType type, JSONLiteral val) : type(type), val(val) {}
  };

  std::vector<Token> tokenize(std::string_view str);

  const char* json_token_type_cstr(TokenType tokenType);
  std::string json_token_type_str(TokenType tokenType);
  std::string json_token_str(Token token);
};


#endif