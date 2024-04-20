#include "jsxxn_impl.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>
#include <utility>

#include <cstddef>
#include <cstdint>


namespace jsxxn {

  JSON::JSON() { this->value = nullptr; }
  JSON::JSON(std::nullptr_t value) { this->value = value; }
  JSON::JSON(bool value) { this->value = value; }
  JSON::JSON(std::int8_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int16_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int32_t value) { this->value = static_cast<std::int64_t>(value); }
  JSON::JSON(std::int64_t value) { this->value = value; }

  JSON::JSON(double value) { this->value = value; }

  JSON::JSON(const char* value) { this->value = std::string(value); }
  JSON::JSON(std::string_view value) { this->value = std::string(value); }
  JSON::JSON(const std::string& value) { this->value = value; }
  JSON::JSON(std::string&& value) { this->value = std::move(value); }

  JSON::JSON(JSONNumber value) { this->value = value; }
  
  JSON::JSON(const JSONLiteral& value) { this->value = value; }
  JSON::JSON(JSONLiteral&& value) { this->value = std::move(value); }
  
  JSON::JSON(const JSONObject& value) { this->value = value; }
  JSON::JSON(JSONObject&& value) { this->value = std::move(value); }

  JSON::JSON(const JSONArray& value) { this->value = value; }
  JSON::JSON(JSONArray&& value) { this->value = std::move(value); }

  JSON::JSON(const JSONValue& value) { this->value = value; }
  JSON::JSON(JSONValue&& value) { this->value = std::move(value); }

  JSON::JSON(const JSON& other) { this->value = other.value; }
  JSON::JSON(JSON&& other) { this->value = std::move(other.value); }

  JSON::JSON(JSONValueType type) {
    switch (type) {
      case JSONValueType::ARRAY: this->value = JSONArray(); break;
      case JSONValueType::OBJECT: this->value = JSONObject(); break;
      case JSONValueType::BOOLEAN: this->value = false; break;
      case JSONValueType::NUMBER: this->value = 0.0; break;
      case JSONValueType::STRING: this->value = std::string(); break;
      case JSONValueType::NULLPTR: this->value = nullptr; break;
    }
  }

  JSON::JSON(JSXXNValueType type) {
    switch (type) {
      case JSXXNValueType::ARRAY: this->value = JSONArray(); break;
      case JSXXNValueType::OBJECT: this->value = JSONObject(); break;
      case JSXXNValueType::BOOLEAN: this->value = false; break;
      case JSXXNValueType::SINTEGER: this->value = 0; break;
      case JSXXNValueType::DOUBLE: this->value = 0.0; break;
      case JSXXNValueType::STRING: this->value = std::string(); break;
      case JSXXNValueType::NULLPTR: this->value = nullptr; break;
    }
  }

  JSON& JSON::operator=(const JSON& other) {
    this->value = other.value;
    return *this;
  }

  JSON& JSON::operator=(JSON&& other) {
    this->value = std::move(other.value);
    return *this;
  }

  JSONValueType JSON::type() const {
    return json_value_get_type(this->value);
  }

  JSXXNValueType JSON::xtype() const {
    return json_value_get_xtype(this->value);
  }

  bool JSON::equals_deep(const JSON& other) const {
    return json_value_equals_deep(this->value, other.value);
  }

  JSON::operator bool() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value))
      if (const bool* boolean = std::get_if<bool>(literal))
        return *boolean;
    throw std::runtime_error("[JSON::operator bool()] cannot cast "
      "non-bool type to bool");
  }

  JSON::operator std::string&() {
    if (JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value))
      if (std::string* str = std::get_if<std::string>(literal))
        return *str;
    throw std::runtime_error("[JSON::operator std::string()] cannot cast "
    " non-string type to string");
  }

  JSON::operator const std::string&() const {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value))
      if (const std::string* str = std::get_if<std::string>(literal))
        return *str;
    throw std::runtime_error("[JSON::operator std::string()] cannot cast "
    " non-string type to string");
  }

  JSON::operator JSONValue&() { return this->value; }
  JSON::operator const JSONValue&() const { return this->value; }

  JSON::operator JSONArray&() {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return *arr;
    throw std::runtime_error("[JSON::operator JSONArray&()] cannot cast "
    "non JSONArray type to JSONArray&");
  }

  JSON::operator const JSONArray&() const {
    if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return *arr;
    throw std::runtime_error("[JSON::operator JSONArray&()] cannot cast "
    "non JSONArray type to JSONArray&");
  }

  JSON::operator JSONObject&() {
    if (JSONObject* arr = std::get_if<JSONObject>(&this->value))
      return *arr;
    throw std::runtime_error("[JSON::operator JSONObject&()] cannot cast "
    "non JSONObject type to JSONObject&");
  }

  JSON::operator const JSONObject&() const {
    if (const JSONObject* arr = std::get_if<JSONObject>(&this->value))
      return *arr;
    throw std::runtime_error("[JSON::operator JSONObject&()] cannot cast "
    "non JSONObject type to JSONObject&");
  }
  
  JSON::operator double() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value))
      if (const JSONNumber* number = std::get_if<JSONNumber>(literal))
        return std::visit(overloaded {
          [](const std::int64_t val) { return static_cast<double>(val); },
          [](const double val) { return (val); },
        }, *number);
    throw std::runtime_error("[JSON::operator double()] cannot cast non-number "
    "type to double");
  }

  JSON::operator std::int64_t() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value))
      if (const JSONNumber* number = std::get_if<JSONNumber>(literal))
        return std::visit(overloaded {
          [](const std::int64_t val) { return val; },
          [](const double val) { return (std::int64_t)val; },
        }, *number);
    throw std::runtime_error("[JSON::operator std::int64_t()] cannot cast "
    "non-number type to std::int64_t");
  }

  JSON::operator std::nullptr_t() {
    if (const JSONLiteral* literal = std::get_if<JSONLiteral>(&this->value))
      if (std::holds_alternative<std::nullptr_t>(*literal))
        return nullptr;
    throw std::runtime_error("[JSON::operator std::nullptr_t()] cannot cast "
    "non-nullptr_t type to nullptr_t");
  }

  bool JSON::empty() const {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value))
      return map->empty();
    else if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->empty();
    throw std::runtime_error("[JSON::empty] queried non-container type");
  }

  std::size_t JSON::size() const {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value))
      return map->size();
    else if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->size();
    throw std::runtime_error("[JSON::size] queried non-container type");
  }

  std::size_t JSON::max_size() const {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value))
      return map->max_size();
    else if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->max_size();
    throw std::runtime_error("[JSON::max_size] queried non-container type");
  }

  void JSON::clear() {
    if (JSONObject* map = std::get_if<JSONObject>(&this->value))
      return map->clear();
    else if (JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->clear();
    throw std::runtime_error("[JSON::clear] tried to clear non-container type");
  }


  // -----------------------------------------------------------
  //                    Object Functions
  // -----------------------------------------------------------

  JSON& JSON::operator[](const std::string& key) {
    if (JSONObject* map = std::get_if<JSONObject>(&this->value))
      return (*map)[key];
    throw std::runtime_error("[JSON::operator[]] searching key on non-object "
    "type");
  }

  JSON& JSON::at(std::string_view key) {
    if (JSONObject* map = std::get_if<JSONObject>(&this->value)) {
      auto iter = (*map).find(key);
      if (iter != map->end()) return iter->second;
      throw std::runtime_error("[JSON::operator[]] could not find key");
    }
    throw std::runtime_error("[JSON::operator[]] searching key on non-object "
    "type");
  }

  const JSON& JSON::at(std::string_view key) const {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value)) {
      auto iter = (*map).find(key);
      if (iter != map->end()) return iter->second;
      throw std::runtime_error("[JSON::operator[]] could not find key");
    }
    throw std::runtime_error("[JSON::operator[]] searching key on non-object "
    "type");
  }

  jsxxn::JSONObject::size_type JSON::count(std::string_view key) const {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value))
      return map->count(key);
    throw std::runtime_error("[JSON::count] searching key on non-object "
    "type");
  }

  bool JSON::contains(std::string_view key) const {
    if (const JSONObject* map = std::get_if<JSONObject>(&this->value))
      return map->count(key) == 1;
    throw std::runtime_error("[JSON::count] searching key on non-object "
    "type");
  }

  template< class... Args >
  std::pair<JSONObject::iterator, bool> JSON::emplace(Args&&... args) {
    if (JSONObject* map = std::get_if<JSONObject>(&this->value))
      return map->emplace(std::forward<Args>(args)...);
    throw std::runtime_error("[JSON::count] emplacing pair on non-object "
    "type");
  }


  // -----------------------------------------------------------
  //                    Array Functions
  // -----------------------------------------------------------


  JSON& JSON::operator[](std::size_t idx) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return (*arr)[idx];
    throw std::runtime_error("[JSON::operator[]] indexing non-array type"); 
  }

  const JSON& JSON::operator[](std::size_t idx) const {
    if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return (*arr)[idx];
    throw std::runtime_error("[JSON::operator[]] indexing non-array type"); 
  }

  JSON& JSON::at(std::size_t idx) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->at(idx);
    throw std::runtime_error("[JSON::operator[]] indexing non-array type"); 
  }

  const JSON& JSON::at(std::size_t idx) const {
    if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->at(idx);
    throw std::runtime_error("[JSON::operator[]] indexing non-array type"); 
  }

  void JSON::push_back(JSON&& json) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      arr->push_back(std::move(json));
      return;
    } else if (std::holds_alternative<JSONLiteral>(this->value)) {
      this->value = JSONArray();
      arr->push_back(std::move(json));
    }
    throw std::runtime_error("[JSON::push_back] pushing on non-array type"); 
  }

  void JSON::push_back(const JSON& json) {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      arr->push_back(json);
      return;
    }
    throw std::runtime_error("[JSON::push_back] pushing on non-array type"); 
  }

  JSON& JSON::front() {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->front();
    throw std::runtime_error("[JSON::front] getting front of non-array type");
  }

  const JSON& JSON::front() const {
    if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->front();
    throw std::runtime_error("[JSON::front] getting front of non-array type");
  } 

  JSON& JSON::back() {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->back();
    throw std::runtime_error("[JSON::back] getting back of non-array type");
  }

  const JSON& JSON::back() const {
    if (const JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->back();
    throw std::runtime_error("[JSON::back] getting back of non-array type");
  }

  void JSON::pop_back() {
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      arr->pop_back();
      return;
    }
    throw std::runtime_error("[JSON::pop_back] popping back of non-array type");
  }

  #if __cplusplus > 201703L
  template< class... Args >
  constexpr JSON& JSON::emplace_back(Args&&... args) {
  #elif __cplusplus == 201703L
  template< class... Args >
  JSON& JSON::emplace_back(Args&&... args) {
  #endif
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value))
      return arr->emplace_back(std::forward<Args>(args)...);
    throw std::runtime_error("[JSON::emplace_back] emplacing in non-array type");
  }

  #if __cplusplus > 201703L
  template< class... Args >
  constexpr JSON& JSON::emplace(std::size_t i, Args&&... args) {
  #elif __cplusplus == 201703L
  template< class... Args >
  JSON& JSON::emplace(std::size_t i, Args&&... args) {
  #endif
    if (JSONArray* arr = std::get_if<JSONArray>(&this->value)) {
      if (i > arr->size())
        throw std::runtime_error("[JSON::emplace] emplace out of bounds");
      return arr->emplace(arr->begin() + i, std::forward<Args>(args)...);
    }
    throw std::runtime_error("[JSON::emplace] emplacing in non-array type");
  }

};