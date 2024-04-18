#ifndef JSXXN_IMPL_H
#define JSXXN_IMPL_H

#include "jsxxn.h"
#define JSXXN_IMPL_MAX_NESTING_DEPTH 250

#include <string_view>
#include <cstddef>

namespace jsxxn {
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
  
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  struct LexState {
    const std::string_view str;
    std::size_t curr;
    const std::size_t size;
    LexState(std::string_view str) : str(str), curr(0), size(str.length()) {}
    LexState(const LexState& ls) : str(ls.str), curr(0), size(ls.size) {}
  };

  struct Token {
    TokenType type;
    TokenLiteral val;
    Token() : type(TokenType::END_OF_FILE), val("EOF") {}
    Token(TokenType type, TokenLiteral val) : type(type), val(val) {}
    Token(const Token& token) : type(token.type), val(token.val) {}
    Token(Token&& token) : type(token.type), val(token.val) {}
    void operator=(const Token& tok) { this->type = tok.type; this->val = tok.val; }
    void operator=(Token&& tok) { this->type = tok.type; this->val = tok.val; }
  };

  std::vector<Token> tokenize(std::string_view str);
  Token nextToken(LexState& state);

  /**
   * assumes a valid json string
  */
  std::string json_string_resolve(std::string_view v);
  
  const char* json_token_type_cstr(TokenType tokenType);
  std::string json_token_type_str(TokenType tokenType);
  std::string json_token_str(Token token);
};


#endif