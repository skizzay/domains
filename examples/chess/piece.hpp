#pragma once

#include "color.hpp"
#include "location.hpp"
#include <system_error>

namespace chess {

class piece {
   location l;
   color c;

public:
   explicit piece(location const initial_location, color team) noexcept : l{initial_location},
                                                                          c{team} {
   }

   location current_location() const noexcept {
      return l;
   }

   color team() const noexcept {
      return c;
   }

   std::error_code move_to(location const new_location) noexcept {
      if ((new_location != current_location()) && can_move_to(new_location)) {
         l = new_location;
         return {};
      }
      return make_error_code(std::errc::result_out_of_range);
   }

   virtual bool can_move_to(location const l) const noexcept = 0;
};

class rook : public piece {
public:
   using piece::piece;

   virtual bool can_move_to(location const l) const noexcept {
      return current_location().row() == l.row() || current_location().column() == l.column();
   }
};

class bishop : public piece {
public:
   using piece::piece;

   virtual bool can_move_to(location const l) const noexcept override {
      using std::abs;
      return abs(current_location().row() - l.row()) ==
             abs(current_location().column() - l.column());
   }
};

class king : public piece {
public:
   using piece::piece;

   virtual bool can_move_to(location const l) const noexcept override {
      using std::abs;
      return (abs(current_location().column() - l.column()) <= 1) &&
             (abs(current_location().row() - l.row()) <= 1);
   }
};

class queen : public piece {
public:
   using piece::piece;

   virtual bool can_move_to(location const l) const noexcept override {
      using std::abs;
      return (current_location().row() == l.row()) || (current_location().column() == l.column()) ||
             (abs(current_location().row() - l.row()) ==
              abs(current_location().column() - l.column()));
   }
};

class pawn : public piece {
   bool first_move = true;

public:
   using piece::piece;

   virtual bool can_move_to(location const l) const noexcept override {
      using std::abs;
      return current_location().column() == l.column() &&
                 (abs(current_location().row() - l.row()) == 1) ||
             ((abs(current_location().row() - l.row()) == 2) && first_move)
   }
};
}
