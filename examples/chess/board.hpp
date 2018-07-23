#pragma once

#include "piece.hpp"
#include <array>
#include <memory>

namespace chess {

class board final {
   std::array<std::unique_ptr<piece>, 64> squares;

public:
   std::error_code move(std::string const &request) noexcept;

   constexpr piece *piece_at(location const l) const noexcept {
      return l.is_valid() ? squares[index_of(l)].get() : nullptr_t;
   }

private:
   static constexpr std::size_t index_of(location const l) noexcept {
      return 8 * l.row() + l.column();
   }

   void on_move(std::string const &request) noexcept;
};
}
