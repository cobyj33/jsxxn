#include "json_impl.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>


namespace json {
  void serialize(const JSONValue& json, unsigned int depth, std::string& output);
  void json_literal_serialize(const JSONLiteral& literal, std::string& output);
  void json_number_serialize(const JSONNumber& number, std::string& output);

  std::string u16_as_hexstr(std::uint16_t val);
  
  std::string serialize(const JSONValue& json) {
    std::string output;
    serialize(json, 0, output);
    return output;
  }

  std::string json_number_serialize(const JSONNumber& number) {
    return std::visit(overloaded {
      [](const std::int64_t& num) { return std::to_string(num); },
      [](const double& num) { return std::to_string(num); }
    }, number);
  }

  std::string json_literal_serialize(const JSONLiteral& literal) {
    std::string output;
    json_literal_serialize(literal, output);
    return output;
  }

  void json_number_serialize(const JSONNumber& number, std::string& output) {
    std::visit(overloaded {
      [&](const std::int64_t& num) { output += std::to_string(num); },
      [&](const double& num) { output += std::to_string(num); }
    }, number);
  }

  void json_string_serialize(std::string_view str, std::string& output) {
    output += "\"";
    for (std::string::size_type i = 0; i < str.size(); i++) {
      switch (str[i]) {
        case '"': output.append("\\\""); break;
        case '\\': output.append("\\\\"); break;
        case '\b': output.append("\\b"); break;
        case '\f': output.append("\\f"); break;
        case '\n': output.append("\\n"); break;
        case '\r': output.append("\\r"); break;
        case '\t': output.append("\\t"); break;
        default: {
          // control characters get turned into unicode escapes
          if (std::iscntrl(str[i])) {
            output += "\\u";
            output += u16_as_hexstr(str[i]);
          } else { // all other characters can just be unescaped
            output.push_back(str[i]);
          }
        }
      }
    }

    output += "\"";
  }

  std::string json_string_serialize(std::string_view v) {
    std::string out;
    json_string_serialize(v, out);
    return out;
  }

  void json_literal_serialize(const JSONLiteral& literal, std::string& output) { 
    std::visit(overloaded {
      [&](const JSONNumber& number) { json_number_serialize(number, output); },
      [&](const std::nullptr_t& nptr) {
        (void)nptr;
        output += "null";
      },
      [&](const bool& boolean) {
        output += boolean ? "true" : "false";
      },
      [&](const std::string& str) {
        json_string_serialize(str, output);
      }
    }, literal);
  }

  void serialize(const JSONValue& json, unsigned int depth, std::string& output) {
    if (depth > JSON_IMPL_MAX_NESTING_DEPTH) {
      throw std::runtime_error("[json::serialize] Exceeded max nesting "
      "depth of " + std::to_string(JSON_IMPL_MAX_NESTING_DEPTH));
    }

    std::visit(overloaded { 
      [&](const JSONLiteral& literal) {
        json_literal_serialize(literal, output);
      },
      [&](const JSONObject& object) {
        if (object.size() == 0) {
          output += "{}";
          return;
        }

        std::string tab(depth * 2, ' ');
        output += "{\n";

        JSONObject::size_type i = 0;
        for (const std::pair<const std::string, JSON>& entry : object) {
          output += tab;
          output += "  ";
          json_literal_serialize(entry.first, output); 
          output += ": "; 
          serialize(entry.second.value, depth + 1, output);
          if (i != object.size() - 1) output += ", ";
          output += "\n";

          i++;
        }

        output += tab;
        output += "}";
      },
      [&](const JSONArray& arr) {
        if (arr.size() == 0) {
          output += "[]";
          return;
        }

        std::string tab(depth * 2, ' ');
        output += "[\n";

        for (JSONArray::size_type i = 0; i < arr.size(); i++) {
          output += tab;
          output += "  ";
          serialize(arr[i].value, depth + 1, output);
          if (i != arr.size() - 1) output += ", ";
          output += "\n";
        }
        output += tab;
        output += "]";
      }
    }, json);
  }

  std::string u16_as_hexstr(std::uint16_t val) {
    std::stringstream res;
    res << std::setfill('0') << std::setw(sizeof(std::uint16_t)*2);
    res << std::hex << val;
    return res.str();
  }

};