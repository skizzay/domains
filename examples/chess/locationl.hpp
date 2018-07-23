#pragma once

namespace chess {
class location {
   char column_;
   char row_;

public:
   constexpr inline explicit location(char column, char row) noexcept : column_(column), row_(row) {
   }

   constexpr inline char column() const noexcept {
      return column_;
   }

   constexpr inline char row() const noexcept {
      return row_;
   }

   constexpr inline bool is_valid() const noexcept {
      return 'A' <= column() && column() <= 'H' && '0' <= row() && row() <= '9';
   }
};
}
