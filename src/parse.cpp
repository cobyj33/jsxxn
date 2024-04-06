#include "json_impl.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>
namespace json {

  struct ParserState {
    std::vector<Token> tokens;
    std::size_t curr;
    const std::size_t size;
    ParserState(std::string_view v) : tokens(tokenize(v)), curr(0), size(v.length()) {}
  };

  JSON parse_value(ParserState& ps, unsigned int depth);
  JSON parse_array(ParserState& ps, unsigned int depth);
  JSON parse_object(ParserState& ps, unsigned int depth);
  void parse_object_pair(ParserState& ps, JSON& obj, unsigned int depth);

  JSON parse(std::string_view str) {
    ParserState ps(str);

    JSON value = parse_value(ps, 0);
    if (ps.curr <= ps.size && ps.tokens[ps.curr].type != TokenType::END_OF_FILE)
        throw std::runtime_error("[json::JSONParser::parse] Did not read "
          "all tokens as a value. ( Next Token: ( " +
          json_token_str(ps.tokens[ps.curr]) + " )");

    return value;
  }

  inline JSONLiteral token_lit_to_json_lit(TokenLiteral literal) {
    
    // if (std::holds_alternative<JSONNumber>(literal)) {
    //   JSONNumber num = std::get<JSONNumber>(literal);
    //   return JSONLiteral(num);
    // } else if (std::holds_alternative<std::string_view>(literal)) {
    //   std::string_view v = std::get<std::string_view>(literal);
    //   return JSONLiteral(std::move(json_string_resolve(v)));
    // } else if (std::holds_alternative<bool>(literal)) {
    //   bool b = std::get<bool>(literal);
    //   return JSONLiteral(b);
    // }

    // return JSONLiteral(nullptr);
    return std::visit(overloaded {
      [](const JSONNumber& number) { return JSONLiteral(number); },
      [](const std::nullptr_t& nptr) { return JSONLiteral(nptr); },
      [](const bool& boolean) { return JSONLiteral(boolean); },
      [](const std::string_view& str) { return JSONLiteral(std::move(json_string_resolve(str))); }
    }, literal);
  }

  JSON parse_value(ParserState& ps, unsigned int depth) {
    if (depth > JSON_IMPL_MAX_NESTING_DEPTH)
      throw std::runtime_error("[json::JSONParser::parse_value] Exceeded "
      "max nesting depth of " + std::to_string(JSON_IMPL_MAX_NESTING_DEPTH));

    switch (ps.tokens[ps.curr].type) {
      case TokenType::LEFT_BRACE: return parse_object(ps, depth);
      case TokenType::LEFT_BRACKET: return parse_array(ps, depth);
      case TokenType::TRUE: ps.curr++; return JSON(true); 
      case TokenType::FALSE: ps.curr++; return JSON(false);
      case TokenType::NULLPTR: ps.curr++; return JSON(nullptr);
      case TokenType::NUMBER: return JSON(token_lit_to_json_lit(ps.tokens[ps.curr++].val));
      case TokenType::STRING: return JSON(token_lit_to_json_lit(ps.tokens[ps.curr++].val));

      case TokenType::END_OF_FILE: throw std::runtime_error("[json::parse] expected value, got END_OF_FILE");

      case TokenType::RIGHT_BRACE: 
      case TokenType::RIGHT_BRACKET:
      case TokenType::COLON:
      case TokenType::COMMA: // error
      default:
        throw std::runtime_error("[json::parse] expected a JSON value, "
        "got invalid token of type " +
        json_token_type_str(ps.tokens[ps.curr].type) +
        " ( " + json_token_str(ps.tokens[ps.curr] ) + " ) ");
    }
  }

  JSON parse_array(ParserState& ps, unsigned int depth) {
    // Array Grammar: "[" (value (, value)* )? "]"

    JSON arr(JSONValueType::ARRAY);
    ps.curr++; //consume opening bracket

    if (ps.tokens[ps.curr].type == TokenType::RIGHT_BRACKET) {
      ps.curr++;
      return arr;
    }

    // We know we must have a value inside of the array now.
    arr.push_back(std::move(parse_value(ps, depth + 1)));

    while (ps.tokens[ps.curr].type != TokenType::RIGHT_BRACKET) {
      switch (ps.tokens[ps.curr].type) {
        case TokenType::END_OF_FILE:
          throw std::runtime_error("[json::JSONParser::parse_array] Unclosed Array. Reached END_OF_FILE");
        case TokenType::COMMA: {
          ps.curr++;
          arr.push_back(std::move(parse_value(ps, depth + 1)));
        } break;
        default: throw std::runtime_error("[json::JSONParser::parse_array] "
          "Unexpected token hit, comma (\",\") or right bracket (\"]\") expected: " +
          json_token_str(ps.tokens[ps.curr]));
      }
    }

    ps.curr++; // consume right bracket
    return arr;
  }


  // Grammar: STRING ":" value
  void parse_object_pair(ParserState& ps, JSON& obj, unsigned int depth) {
    if (ps.tokens[ps.curr].type != TokenType::STRING)
      throw std::runtime_error("[parse_object_pair] found object key " +
      json_token_str(ps.tokens[ps.curr]) + ". String expected.");

    std::string_view raw_key = std::get<std::string_view>(ps.tokens[ps.curr].val);
    std::string key = json_string_resolve(raw_key);
    ps.curr++;

    if (ps.tokens[ps.curr].type != TokenType::COLON)
      throw std::runtime_error("[parse_object_pair] expected colon, got " + json_token_str(ps.tokens[ps.curr]));

    ps.curr++;
    JSONObject& objval = std::get<JSONObject>(obj.value);
    objval.emplace(std::move(key), std::move(parse_value(ps, depth + 1)));
  }

  JSON parse_object(ParserState& ps, unsigned int depth) {
    // Object Grammar: "{" ( ( STRING ":" value ) (, STRING ":"" value)* )? "}"
    JSON obj(JSONValueType::OBJECT);

    ps.curr++;
    if (ps.tokens[ps.curr].type == TokenType::RIGHT_BRACE) {
      ps.curr++;
      return obj;
    }

    parse_object_pair(ps, obj, depth);

    while (ps.tokens[ps.curr].type != TokenType::RIGHT_BRACE) {
      switch (ps.tokens[ps.curr].type) {
        case TokenType::END_OF_FILE:
          throw std::runtime_error("[json::parse_object] unclosed object");
        case TokenType::COMMA: {
          ps.curr++;
          parse_object_pair(ps, obj, depth);
        } break;
        default: throw std::runtime_error("[json::parse_object] "
          "Unexpected separator token of type " +
          json_token_type_str(ps.tokens[ps.curr].type) +
          ", expected colon (\"'\"). ( " + json_token_str(ps.tokens[ps.curr]) + " )");
      }
    }

    ps.curr++; // consume right brace
    return obj;
  }
};