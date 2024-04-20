#ifndef JSXXN_COBYJ33_H
#define JSXXN_COBYJ33_H

#include <string>
#include <vector>
#include <string_view>
#include <variant>
#include <map>
#include <cstddef>
#include <utility>

// [ { "name": 3 }, { "age": 4 }, [ 3, 5, 8 ], "String" ]

namespace jsxxn {
  class JSON;

  typedef std::int64_t s64;
  typedef std::uint64_t u64;

  typedef std::variant<std::int64_t, double> JSONNumber;
  typedef std::variant<std::nullptr_t, std::string, JSONNumber, bool> JSONLiteral;
  typedef std::map<std::string, JSON, std::less<>> JSONObject;
  typedef std::vector<JSON> JSONArray;

  typedef std::variant<JSONLiteral, JSONObject, JSONArray> JSONValue;

  typedef std::string JSONSerializeFunc(const jsxxn::JSONValue& value);

  enum class JSONValueType {
    OBJECT,
    ARRAY,
    NUMBER,
    BOOLEAN,
    STRING,
    NULLPTR
  };

  enum class JSXXNValueType {
    OBJECT,
    ARRAY,
    SINTEGER,
    DOUBLE,
    BOOLEAN,
    STRING,
    NULLPTR
  };

  JSXXNValueType json_number_get_xtype(const JSONNumber& num);
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
  JSXXNValueType json_literal_get_xtype(const JSONLiteral& value);
  JSONValueType json_value_get_type(const JSONValue& value);
  JSXXNValueType json_value_get_xtype(const JSONValue& value);
  
  JSONValueType jsxxnt_to_jsont(JSXXNValueType jsxnvt);
  const char* jsonvt_str(JSONValueType jvt);
  const char* jsxxnvt_str(JSXXNValueType jsxnvt);
  bool json_value_equals_deep(const JSONValue& a, const JSONValue& b);

  std::string json_string_serialize(std::string_view v);
  std::string json_literal_serialize(const JSONLiteral& literal);
  std::string json_number_serialize(const JSONNumber& number);

  std::string stringify(const JSONValue& json);
  std::string prettify(const JSONValue& json);
  JSON parse(std::string_view str);

  class JSON {
    public:
      JSONValue value;

      JSON(); // defaults to hold null
      JSON(JSONValueType type);
      JSON(JSXXNValueType type);
      JSON(std::nullptr_t value);
      JSON(bool value);
      JSON(std::int8_t value);
      JSON(std::int16_t value);
      JSON(std::int32_t value);
      JSON(std::int64_t value);
      JSON(double value);
      JSON(const char* value);
      JSON(std::string_view value);
      explicit JSON(const std::string& value);
      explicit JSON(std::string&& value);
      explicit JSON(JSONNumber value);
      explicit JSON(const JSONLiteral& value);
      explicit JSON(JSONLiteral&& value);
      JSON(const JSONArray& value);
      JSON(JSONArray&& value);
      JSON(const JSONObject& value);
      JSON(JSONObject&& value);
      JSON(const JSON& value);
      JSON(JSON&& value);

      // explicit to be unambiguous with const JSON& and JSON&&
      explicit JSON(const JSONValue& value);
      explicit JSON(JSONValue&& value);

      bool equals_deep(const JSON& other) const;
      
      JSONValueType type() const;
      JSXXNValueType xtype() const;

      JSON& operator=(const JSON& other);
      JSON& operator=(JSON&& other);

      // note that directly defining assignment operators for JSONValue causes
      // an ambiguous overload conflict with assigning simple values like string
      // litearls to JSON objects, since JSON::JSON(const JSONValue&) and
      // JSON::JSON(JSONValue&&) exist.
      // JSON& operator=(const JSONValue& value);
      // JSON& operator=(JSONValue&& value);


      // Literal Methods
      explicit operator bool();
      explicit operator std::string&();
      explicit operator double();
      explicit operator std::int64_t();
      explicit operator std::nullptr_t();
      operator JSONValue&();
      operator const JSONValue&() const;
      explicit operator JSONArray&();
      explicit operator const JSONArray&() const;
      explicit operator JSONObject&();
      explicit operator const JSONObject&() const;

      bool empty() const;
      std::size_t size() const;
      std::size_t max_size() const;
      void clear();
      
      // Array Methods
      void push_back(const JSON& json);
      void push_back(JSON&& json);
      JSON& operator[](std::size_t idx);
      const JSON& operator[](std::size_t idx) const;
      JSON& at(std::size_t idx);
      const JSON& at(std::size_t idx) const;
      void pop_back();
      JSON& front();
      const JSON& front() const;
      JSON& back();
      const JSON& back() const;

      #if __cplusplus > 201703L
      template< class... Args >
      constexpr JSON& emplace_back(Args&&... args);
      #elif __cplusplus == 201703L
      template< class... Args >
      JSON& emplace_back(Args&&... args);
      #endif

      #if __cplusplus > 201703L
      template< class... Args >
      constexpr JSON& emplace(std::size_t i, Args&&... args);
      #elif __cplusplus == 201703L
      template< class... Args >
      JSON& emplace(std::size_t i, Args&&... args);
      #endif


      // Object Methods
      JSONObject::size_type count(std::string_view key) const;
      bool contains(std::string_view key) const;
      JSON& operator[](const std::string& key);
      JSON& at(std::string_view key);
      const JSON& at(std::string_view key) const;
      
      template< class... Args >
      std::pair<JSONObject::iterator, bool> emplace(Args&&... args);
      
  };
};

#endif