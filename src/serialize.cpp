#include "jsxxn_impl.h"

#include "jsxxn_string.h"

#include <stdexcept>
#include <sstream>


namespace jsxxn {
  void prettify(const JSONValue& json, unsigned int depth, std::string& output);
  void stringify(const JSONValue& json, unsigned int depth, std::string& output);
  void json_literal_serialize(const JSONLiteral& literal, std::string& output);
  void json_number_serialize(const JSONNumber& number, std::string& output);
  void json_string_serialize(std::string_view str, std::string& output);

  inline void u16_as_hexstr(std::uint16_t val, std::string& output) {
    output.push_back(xdigit_as_ch((val & 0xF000) >> 12));
    output.push_back(xdigit_as_ch((val & 0x0F00) >> 8));
    output.push_back(xdigit_as_ch((val & 0x00F0) >> 4));
    output.push_back(xdigit_as_ch(val & 0x000F));
  }
  
  std::string prettify(const JSONValue& json) {
    std::string output;
    prettify(json, 0, output);
    return output;
  }

  std::string stringify(const JSONValue& json) {
    std::string output;
    stringify(json, 0, output);
    return output;
  }

  std::string json_number_serialize(const JSONNumber& number) {
    return std::visit(overloaded {
      [](const std::int64_t num) { return std::to_string(num); },
      [](const double num) { return std::to_string(num); }
    }, number);
  }

  void json_number_serialize(const JSONNumber& number, std::string& output) {
    // we end up having to allocate a string anyway while serializing the
    // numbers... so we can just have the result of the non-capturing 
    // std::visit append directly onto the output string

    output += std::visit(overloaded {
      [](const std::int64_t num) { return std::to_string(num); },
      [](const double num) { return std::to_string(num); }
    }, number);
  }

  std::string json_literal_serialize(const JSONLiteral& literal) {
    std::string output;
    json_literal_serialize(literal, output);
    return output;
  }

  void json_string_serialize(std::string_view str, std::string& output) {
    output.push_back('\"');

    for (std::string_view::size_type i = 0; i < str.size(); i++) {
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
          if (std::iscntrl(str[i]) || str[i] == 127) {
            output += "\\u";
            u16_as_hexstr(static_cast<std::uint16_t>(str[i]), output);
          } else { // all other unicode characters can just be unescaped
            output.push_back(str[i]);
          }
        }
      }
    }

    output.push_back('\"');
  }

  std::string json_string_serialize(std::string_view v) {
    std::string out;
    json_string_serialize(v, out);
    return out;
  }

  void json_literal_serialize(const JSONLiteral& literal, std::string& output) { 
    std::visit(overloaded {
      [&output](const JSONNumber& number) { json_number_serialize(number, output); },
      [&output](const std::nullptr_t nptr) {
        (void)nptr;
        output += "null";
      },
      [&output](const bool boolean) {
        output += boolean ? "true" : "false";
      },
      [&output](const std::string& str) {
        json_string_serialize(str, output);
      }
    }, literal);
  }

  void prettify(const JSONValue& json, unsigned int depth, std::string& output) {
    if (depth > JSXXN_IMPL_MAX_NESTING_DEPTH) {
      throw std::runtime_error("[jsxxn::prettify] Exceeded max nesting "
      "depth of " + std::to_string(JSXXN_IMPL_MAX_NESTING_DEPTH));
    }

    std::visit(overloaded { 
      [&output](const JSONLiteral& literal) {
        json_literal_serialize(literal, output);
      },
      [&output, depth](const JSONObject& object) {
        if (object.size() == 0) {
          output += "{}";
          return;
        }
        output += "{\n";

        JSONObject::size_type i = 0;
        for (const std::pair<const std::string, JSON>& entry : object) {
          output.append((depth + 1) * 2, ' ');
          json_string_serialize(entry.first, output); 
          output += ": "; 
          prettify(entry.second.value, depth + 1, output);
          if (i != object.size() - 1) output += ", ";
          output.push_back('\n');
          i++;
        }

        output.append(depth * 2, ' ');
        output.push_back('}');
      },
      [&output, depth](const JSONArray& arr) {
        if (arr.size() == 0) {
          output += "[]";
          return;
        }

        output += "[\n";
        for (JSONArray::size_type i = 0; i < arr.size(); i++) {
          output.append((depth + 1) * 2, ' ');
          prettify(arr[i].value, depth + 1, output);
          if (i != arr.size() - 1) output += ", ";
          output.push_back('\n');
        }

        output.append(depth * 2, ' ');
        output.push_back(']');
      }
    }, json);
  }

  void stringify(const JSONValue& json, unsigned int depth, std::string& output) {
    if (depth > JSXXN_IMPL_MAX_NESTING_DEPTH) {
      throw std::runtime_error("[jsxxn::prettify] Exceeded max nesting "
      "depth of " + std::to_string(JSXXN_IMPL_MAX_NESTING_DEPTH));
    }

    std::visit(overloaded { 
      [&output](const JSONLiteral& literal) {
        json_literal_serialize(literal, output);
      },
      [&output, depth](const JSONObject& object) {
        output.push_back('{');

        for (const std::pair<const std::string, JSON>& entry : object) {
          json_string_serialize(entry.first, output); 
          output.push_back(':'); 
          stringify(entry.second.value, depth + 1, output);
          output.push_back(',');
        }

        if (output[output.size() - 1] == ',')
          output.resize(output.size() - 1);
        output.push_back('}');
      },
      [&output, depth](const JSONArray& arr) {
        output.push_back('[');

        for (JSONArray::size_type i = 0; i < arr.size(); i++) {
          stringify(arr[i].value, depth + 1, output);
          output.push_back(',');
        }

        if (output[output.size() - 1] == ',')
          output.resize(output.size() - 1);
        output.push_back(']');
      }
    }, json);
  }

};