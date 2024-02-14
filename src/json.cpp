#include "json_impl.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>

#include <cstddef>
#include <cstdint>


namespace json {

  JSON::JSON() { this->value = nullptr; }
  JSON::JSON(nullptr_t value) { this->value = value; }
  JSON::JSON(bool value) { this->value = value; }
  JSON::JSON(std::int8_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int16_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int32_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int64_t value) { this->value = value; }

  JSON::JSON(double value) { this->value = value; }
  

  JSON::JSON(const char* value) { this->value = std::string(value); }
  JSON::JSON(std::string_view value) { this->value = std::string(value); }
  JSON::JSON(std::string value) { this->value = value; }
  JSON::JSON(const std::string& value) { this->value = value; }
  JSON::JSON(std::string&& value) { this->value = value; }

  JSON::JSON(JSONNumber value) { this->value = value; }
  
  JSON::JSON(JSONLiteral value) { this->value = value; }
  
  JSON::JSON(const JSON& value) { this->value = value.value; }

  JSON::JSON(const JSONArray& value) { this->value = value; }

  JSON::JSON(JSONArray&& value) { this->value = value; }

  JSON::JSON(const JSONObject& value) { this->value = value; }

  JSON::JSON(JSONObject&& value) { this->value = value; }

  JSON::JSON(JSONValueType type) {
    switch (type) {
      case JSONValueType::ARRAY: this->value = JSONArray(); break;
      case JSONValueType::OBJECT: this->value = JSONObject(); break;
      case JSONValueType::BOOLEAN: this->value = false; break;
      case JSONValueType::NUMBER: this->value = 0; break;
      case JSONValueType::STRING: this->value = std::string(); break;
      case JSONValueType::NULLPTR: this->value = nullptr; break;
    }
  }

  JSONValueType JSON::type() {
    return json_value_get_type(this->value);
  }

  bool JSON::equals_deep(const JSON& other) {
    return json_value_equals_deep(this->value, other.value);
  }

  JSON& JSON::operator=(const JSON& other) {
    this->value = other.value;
    return *this;
  }

  JSON::operator bool() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const bool* boolean = std::get_if<bool>(literal)) {
        return *boolean;
      }
    }
    throw std::runtime_error("[JSON::operator bool()]");

  }

  JSON::operator std::string() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const std::string* str = std::get_if<std::string>(literal)) {
        return *str;
      }
    }
    throw std::runtime_error("[JSON::operator std::string()]");
  }

  JSON::operator JSONValue() { return this->value; }
  JSON::operator JSONValue&() { return this->value; }
  
  JSON::operator double() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const JSONNumber* number = std::get_if<JSONNumber>(literal)) {
        return std::visit(overloaded {
          [&](const std::int64_t val) { return (double)(val); },
          [&](const double val) { return (val); },
        }, *number);
      }
    }
    throw std::runtime_error("[JSON::operator double()]");
  }

  JSON::operator std::int64_t() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (const JSONNumber* number = std::get_if<JSONNumber>(literal)) {
        return std::visit(overloaded {
          [&](const std::int64_t val) { return val; },
          [&](const double val) { return (std::int64_t)val; },
        }, *number);
      }
    }
    throw std::runtime_error("[JSON::operator std::int64_t()]");
  }

  JSON::operator nullptr_t() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value)) {
      if (std::holds_alternative<nullptr_t>(*literal)) {
        return nullptr;
      }
    }
    throw std::runtime_error("[JSON::operator nullptr_t()]");
  }

  JSON JSON::operator[](std::string_view key) {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value)) {
      if (map->count(key) == 1) {
        std::string key_str = std::string(key);
        return map->at(key_str);
      }
      throw std::runtime_error("[JSON::operator[]] key " + std::string(key) + " not found"); 
    }
    throw std::runtime_error("[JSON::operator[]] searching key on non-object type");
  }

  JSON JSON::operator[](const char* key) {
    return (*this)[std::string_view(key)];
  }
  
  JSON JSON::operator[](std::size_t idx) {
    if (const JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      if (idx >= arr->size()) {
        throw std::runtime_error("[JSON::operator[]] index out of range"); 
      }
      return arr->at(idx);
    }
    throw std::runtime_error("[JSON::operator[]] indexing non-array type"); 
  }

  void JSON::push_back(JSON&& json) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      arr->push_back(json);
      return;
    }
    throw std::runtime_error("[JSON::operator[]] pushing on non-array type"); 
  }

  void JSON::push_back(const JSON& json) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      arr->push_back(json);
      return;
    }
    throw std::runtime_error("[JSON::operator[]] pushing on non-array type"); 
  }

};