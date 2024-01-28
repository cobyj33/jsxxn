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
  typedef std::variant<nullptr_t, std::string, JSONNumber, bool> JSONLiteral;
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

  bool json_literal_equals_deep(const JSONLiteral& a, const JSONLiteral& b);
  std::string json_literal_serialize(JSONLiteral literal);
  JSONValueType json_literal_get_type(const JSONLiteral& value);

  JSONValueType json_value_get_type(const JSONValue& value);
  const char* json_value_type_str(JSONValueType jvt);
  bool json_value_equals_deep(const JSONValue& a, const JSONValue& b);

  std::string serialize(const JSON& json);
  JSON parse(std::string_view str);

  class JSON {
    public:
      JSONValue value;

      JSON();

      JSON(JSONValueType type);
      
      JSON(nullptr_t value);

      JSON(bool value);

      JSON(std::int8_t value);
      JSON(std::int16_t value);
      JSON(std::int32_t value);
      JSON(std::int64_t value);
      JSON(double value);
      
      JSON(const char* value);
      JSON(std::string_view value);
      JSON(std::string value);
      JSON(const std::string& value);
      JSON(std::string&& value);

      JSON(JSONNumber value);
      JSON(JSONLiteral value);
      
      JSON(const JSONArray& value);
      JSON(JSONArray&& value);

      JSON(const JSONObject& value);
      JSON(JSONObject&& value);

      JSON(const JSON& value);

      bool equals_deep(const JSON& other);
      
      JSONValueType type();

      JSON& operator=(const JSON& other);

      // Literal Methods
      operator bool();
      operator std::string();
      operator double();
      operator std::int64_t();
      operator nullptr_t();
      
      // Array Methods
      bool empty() const noexcept;
      void push_back(JSON&& json);
      void push_back(const JSON& json);
      JSON operator[](std::size_t idx);

      // Object Methods
      bool containsKey(std::string_view key);
      bool containsValue(std::string_view key);
      JSON operator[](std::string_view key);
      JSON operator[](const char* key);
  };
};

#endif