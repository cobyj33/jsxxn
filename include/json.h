#ifndef JSON_COBYJ33_H
#define JSON_COBYJ33_H

#include <string>
#include <vector>
#include <string_view>
#include <variant>
#include <map>
#include <cstddef>
#include <utility>

// [ { "name": 3 }, { "age": 4 }, [ 3, 5, 8 ], "String" ]

namespace json {
  class JSON;

  typedef std::variant<std::int64_t, double> JSONNumber;
  typedef std::variant<std::nullptr_t, std::string, JSONNumber, bool> JSONLiteral;
  typedef std::map<std::string, JSON, std::less<>> JSONObject;
  typedef std::vector<JSON> JSONArray;

  typedef std::variant<JSONLiteral, JSONObject, JSONArray> JSONValue;


  enum class JSONValueType {
    OBJECT,
    ARRAY,
    NUMBER,
    BOOLEAN,
    STRING,
    NULLPTR
  };

  bool json_number_equals_deep(const JSONNumber& a, const JSONNumber& b);
  // equating JSONNumber's trivially is tricky, since equating doubles should
  // really be done with an epsilon value in mind
  // inline bool json_number_equals_deep(const JSONNumber& a, const JSONNumber& b) { return a == b; };

  bool json_literal_equals_deep(const JSONLiteral& a, const JSONLiteral& b);
  // Since equating JSONNumber's trivially is tricky, equating JSONLiteral's trivially
  // is also tricky
  // inline bool json_literal_equals_deep(const JSONLiteral& a, const JSONLiteral& b) { return a == b; };
  std::string json_literal_serialize(const JSONLiteral& literal);

  JSONValueType json_literal_get_type(const JSONLiteral& value);
  JSONValueType json_value_get_type(const JSONValue& value);
  
  const char* json_value_type_str(JSONValueType jvt);
  bool json_value_equals_deep(const JSONValue& a, const JSONValue& b);

  std::string json_string_serialize(std::string_view v);
  std::string json_literal_serialize(const JSONLiteral& literal);
  std::string json_number_serialize(const JSONNumber& number);

  std::string stringify(const JSONValue& json);
  std::string serialize(const JSONValue& json);
  JSON parse(std::string_view str);

  class JSON {
    public:
      JSONValue value;

      JSON(); // defaults to hold null
      JSON(JSONValueType type);
      JSON(std::nullptr_t value);
      JSON(bool value);
      JSON(std::int8_t value);
      JSON(std::int16_t value);
      JSON(std::int32_t value);
      JSON(std::int64_t value);
      JSON(double value);
      JSON(const char* value);
      JSON(std::string_view value);
      JSON(const std::string& value);
      JSON(std::string&& value);
      JSON(JSONNumber value);
      JSON(const JSONLiteral& value);
      JSON(JSONLiteral&& value);
      JSON(const JSONArray& value);
      JSON(JSONArray&& value);
      JSON(const JSONObject& value);
      JSON(JSONObject&& value);
      JSON(const JSON& value);
      JSON(JSON&& value);

      explicit JSON(const JSONValue& value);
      explicit JSON(JSONValue&& value);

      bool equals_deep(const JSON& other);
      
      JSONValueType type();

      JSON& operator=(const JSON& other);
      JSON& operator=(JSON&& other);

      // note that directly defining assignment operators for JSONValue causes
      // an ambiguous overload conflict with assigning simple values like string
      // litearls to JSON objects, since JSON::JSON(const JSONValue&) and
      // JSON::JSON(JSONValue&&) exist.
      // JSON& operator=(const JSONValue& value);
      // JSON& operator=(JSONValue&& value);


      // Literal Methods
      operator bool();
      operator std::string&();
      operator double();
      operator std::int64_t();
      operator std::nullptr_t();
      operator JSONValue&();
      operator JSONArray&();
      operator JSONObject&();

      bool empty() const;
      std::size_t size() const;
      std::size_t max_size() const;
      void clear();
      
      // Array Methods
      void push_back(const JSON& json);
      void push_back(JSON&& json);
      JSON& operator[](std::size_t idx);
      JSON& at(std::size_t idx);

      // Object Methods
      json::JSONObject::size_type count(const std::string& key) const;
      json::JSONObject::size_type count(std::string_view key) const;
      bool contains(const std::string& key) const;
      bool contains(std::string_view key) const;
      bool containsValue(std::string_view key) const;
      JSON& operator[](const std::string& key);
      JSON& operator[](std::string_view key);
      JSON& at(const std::string& key);
      JSON& at(std::string_view key);
  };
};

#endif