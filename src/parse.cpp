#include "jsxxn_impl.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>
namespace jsxxn {

  struct ParserState {
    LexState ls;
    Token token;
    ParserState(std::string_view v) : ls(LexState(v)) {
      this->token = nextToken(this->ls); // fetches first token!
    }

    void next() {
      this->token = nextToken(this->ls);
    }
  };

  std::string err_not_single_val(Token nextToken);
  std::string err_max_nest();
  std::string err_expect_json_val(Token token);
  std::string err_expect_colon(Token token);
  std::string err_expect_str_key(Token token);
  std::string err_unclsed_arr();
  std::string err_unclsed_obj();
  std::string err_unex_sep_token(Token token);
  std::string err_unex_arr_token(Token token);
  std::string err_got_eof();

  JSON parse_value(ParserState& ps, unsigned int depth);
  JSON parse_array(ParserState& ps, unsigned int depth);
  JSON parse_object(ParserState& ps, unsigned int depth);
  void parse_object_pair(ParserState& ps, JSON& obj, unsigned int depth);

  JSON parse(std::string_view str) {
    ParserState ps(str);

    JSON value = parse_value(ps, 0);
    if (ps.ls.curr < ps.ls.size)
        throw std::runtime_error(err_not_single_val(nextToken(ps.ls)));

    return value;
  }

  inline JSONLiteral token_lit_to_json_lit(TokenLiteral literal) {
    // not sure if if-chain is faster than std::visit with non-capturing lambdas

    #if 0
    if (std::holds_alternative<JSONNumber>(literal)) {
      JSONNumber num = std::get<JSONNumber>(literal);
      return JSONLiteral(num);
    } else if (std::holds_alternative<std::string_view>(literal)) {
      std::string_view v = std::get<std::string_view>(literal);
      return JSONLiteral(json_string_resolve(v));
    } else if (std::holds_alternative<bool>(literal)) {
      bool b = std::get<bool>(literal);
      return JSONLiteral(b);
    }

    return JSONLiteral(nullptr);
    #endif

    return std::visit(overloaded {
      [](const JSONNumber number) { return JSONLiteral(number); },
      [](const std::nullptr_t nptr) { return JSONLiteral(nptr); },
      [](const bool boolean) { return JSONLiteral(boolean); },
      [](const std::string_view str) { return JSONLiteral(json_string_resolve(str)); }
    }, literal);
  }

  JSON parse_value(ParserState& ps, unsigned int depth) {
    if (depth > JSXXN_IMPL_MAX_NESTING_DEPTH)
      throw std::runtime_error(err_max_nest());
    
    switch (ps.token.type) {
      case TokenType::LEFT_BRACE: return parse_object(ps, depth);
      case TokenType::LEFT_BRACKET: return parse_array(ps, depth);
      case TokenType::TRUE: ps.next(); return JSON(true); 
      case TokenType::FALSE: ps.next(); return JSON(false);
      case TokenType::NULLPTR: ps.next(); return JSON(nullptr);
      case TokenType::NUMBER: 
      case TokenType::STRING: {
        TokenLiteral literal = ps.token.val;
        ps.next();
        return JSON(token_lit_to_json_lit(literal));
      }
      case TokenType::END_OF_FILE: throw std::runtime_error(err_got_eof());
      case TokenType::RIGHT_BRACE: 
      case TokenType::RIGHT_BRACKET:
      case TokenType::COLON:
      case TokenType::COMMA: // error
      default:
        throw std::runtime_error(err_expect_json_val(ps.token));
    }
  }

  JSON parse_array(ParserState& ps, unsigned int depth) {
    // Array Grammar: "[" (value (, value)* )? "]"

    JSON arr(JSONValueType::ARRAY);

    ps.next(); // consume left bracket
    if (ps.token.type == TokenType::RIGHT_BRACKET) {
      ps.next(); // consume right bracket 
      return arr; 
    }

    // We know we must have a value inside of the array now.
    arr.push_back(parse_value(ps, depth + 1));

    while (ps.token.type != TokenType::RIGHT_BRACKET) {
      switch (ps.token.type) {
        case TokenType::COMMA: {
          ps.next(); // consume comma
          arr.push_back(parse_value(ps, depth + 1));
        } break;
        case TokenType::END_OF_FILE:
          throw std::runtime_error(err_unclsed_arr());
        default: throw std::runtime_error(err_unex_arr_token(ps.token));
      }
    }

    ps.next(); // consume right bracket
    return arr;
  }


  // Grammar: STRING ":" value
  void parse_object_pair(ParserState& ps, JSON& obj, unsigned int depth) {
    if (ps.token.type != TokenType::STRING)
      throw std::runtime_error(err_expect_str_key(ps.token));

    std::string_view raw_key = std::get<std::string_view>(ps.token.val);
    std::string key = json_string_resolve(raw_key);
    ps.next();

    if (ps.token.type != TokenType::COLON)
      throw std::runtime_error(err_expect_colon(ps.token));

    ps.next(); // consume colon
    JSONObject& objval = std::get<JSONObject>(obj.value);
    objval.emplace(std::move(key), parse_value(ps, depth + 1));
  }

  JSON parse_object(ParserState& ps, unsigned int depth) {
    // Object Grammar: "{" ( ( STRING ":" value ) (, STRING ":"" value)* )? "}"
    JSON obj(JSONValueType::OBJECT);

    ps.next(); // consume left curly brace
    if (ps.token.type == TokenType::RIGHT_BRACE) {
      ps.next(); // consume right curly brace
      return obj;
    }

    parse_object_pair(ps, obj, depth);

    while (ps.token.type != TokenType::RIGHT_BRACE) {
      switch (ps.token.type) {
        case TokenType::COMMA: {
          ps.next(); // consume comma
          parse_object_pair(ps, obj, depth);
        } break;
        case TokenType::END_OF_FILE:
          throw std::runtime_error(err_unclsed_obj());
        default: throw std::runtime_error(err_unex_sep_token(ps.token));
      }
    }

    ps.next(); // consume right brace
    return obj;
  }

  std::string err_not_single_val(Token nextToken) {
    return "Did not read all tokens as a value. ( Next Token: ( "
      + json_token_str(nextToken) + " )";
  }

  std::string err_max_nest() {
    return "Exceeded max nesting depth of " + 
      std::to_string(JSXXN_IMPL_MAX_NESTING_DEPTH);
  }

  std::string err_expect_json_val(Token token) {
    return "Expected a JSON value, "
        "got invalid token of type " +
        json_token_type_str(token.type) +
        " ( " + json_token_str(token ) + " ) ";
  }

  std::string err_unclsed_arr() {
    return "Unclosed Array. Reached END_OF_FILE";
  }

  std::string err_unclsed_obj() {
    return "Unclosed Object. Reached END_OF_FILE";
  }

  std::string err_unex_sep_token(Token token) {
    return "Unexpected separator token of type " +
          json_token_type_str(token.type) +
          ", expected colon (\"'\"). ( " + json_token_str(token) + " )";
  }

  std::string err_expect_colon(Token token) {
    return "expected colon, got " + json_token_str(token);
  }

  std::string err_expect_str_key(Token token) {
    return "found object key " +
      json_token_str(token) + ". String expected.";
  }

  std::string err_got_eof() {
    return "expected value, got END_OF_FILE";
  }

  std::string err_unex_arr_token(Token token) {
    return "Unexpected token hit, comma (\",\") or right bracket (\"]\") "
      "expected: " + json_token_str(token);
  }

};