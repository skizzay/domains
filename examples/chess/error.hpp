#pragma once

#include <system_error>

namespace chess {
enum class move_result_code : int { success, illegal_location, illegal_move, illegal_turn };

class error_category : public std::error_category {
public:
   virtual const char *name() const noexcept override;
   virtual std::string message(int condition) const noexcept override;
};

std::error_code make_error_code(move_result_code c) {
   static error_category singleton;
   return {static_cast<int>(c), singleton};
}
}

namespace std {
template <>
struct is_error_code_enum<chess::move_result_code> : std::true_type {};
template <>
struct is_error_condition_enum<chess::move_result_code> : std::true_type {};
}
