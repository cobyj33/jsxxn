
#include "json_impl.h"

#include <stdexcept>

namespace json {

  bool json_value_equals_deep(const JSONValue& a, const JSONValue& b) {
    return std::visit(overloaded {
      [](const JSONLiteral& literal1, const JSONLiteral& literal2) {
        return json_literal_equals_deep(literal1, literal2);
      },
      [](const JSONObject& obj1, const JSONObject& obj2) {
        if (obj1.size() != obj2.size()) return false;
        for (auto& entry : obj1) {
          if (obj2.count(entry.first) == 0) return false;
          if (!json_value_equals_deep(entry.second.value, obj2.at(entry.first).value))
            return false;
        }
        return true;
      },
      [](const JSONArray& arr1, const JSONArray& arr2) {
        if (arr1.size() != arr2.size()) return false;
        for (JSONArray::size_type i = 0; i < arr1.size(); i++) {
          if (!json_value_equals_deep(arr1[i].value, arr2[i].value))
            return false;
        }
        return true;
      },
      [](const auto& a1, const auto& a2) {
        return false;
        (void)a1; (void)a2;
      }
    }, a, b);
  }  

  // bool json_number_equals_deep(const JSONNumber& a, const JSONNumber& b) {
  //   constexpr double DOUBLE_EPSILON = 1E-6;

  //   return std::visit(overloaded {
  //     [](const std::int64_t& a1, const std::int64_t& b1) { return a1 == b1; },
  //     [](const std::int64_t& a1, const double& b1) {
  //       return std::abs(static_cast<double>(a1) - b1) < DOUBLE_EPSILON;
  //     },
  //     [](const double& a1, const std::int64_t& b1) {
  //       return std::abs(a1 - static_cast<double>(b1)) < DOUBLE_EPSILON;
  //     },
  //     [](const double& a1, const double& b1) {
  //       return std::abs(a1 - b1) < DOUBLE_EPSILON;
  //     }
  //   }, a, b); 
  // }


  // bool json_literal_equals_deep(const JSONLiteral& a, const JSONLiteral& b) {
  //   return std::visit(overloaded {
  //     [](const JSONNumber& a1, const JSONNumber& b1) {
  //       return json_number_equals_deep(a1, b1);  
  //     },
  //     [](const std::nullptr_t& a, const std::nullptr_t& b) {
  //       (void)a; (void)b;
  //       return true;
  //     },
  //     [](const bool& a, const bool& b) {
  //       return a == b;
  //     },
  //     [](const std::string& a, const std::string& b) {
  //       return a == b;
  //     },
  //     [](const auto& a, const auto& b) {
  //       (void)a; (void)b;
  //       return false;
  //     }
  //   }, a, b);
  // }
};