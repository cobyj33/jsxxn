#include "json_impl.h"

#include <stdexcept>

namespace json {
  class JSONParser {

    public:
      JSONParser() {}

      JSON parse(std::string_view str) {
        this->tokens = tokenize(str);

        this->curr = 0;
        JSON value = this->parse_value(0);
        if (this->curr <= this->tokens.size() &&
            this->tokens[this->curr].type != TokenType::END_OF_FILE)
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
          case TokenType::NULLPTR:
          case TokenType::NUMBER:
          case TokenType::STRING: return JSON(this->tokens[this->curr++].val);

          case TokenType::END_OF_FILE: throw std::runtime_error("[json::parse] expected value, got END_OF_FILE");

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
            case TokenType::END_OF_FILE:
              throw std::runtime_error("[json::JSONParser::parse_array] Unclosed Array. Reached END_OF_FILE");
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
            case TokenType::END_OF_FILE:
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
};