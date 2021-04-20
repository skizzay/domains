#pragma once

#include <skizzay/domains/aggregate_root.h>
#include <skizzay/domains/event_source/commit.h>
#include <skizzay/domains/event_source/event_store.h>
#include <exception>
#include <iterator>
#include <optional>

namespace skizzay::domains {

inline constexpr auto retry_on_concurrency_collisions = [](auto command_handler, auto apply_back_pressure, std::size_t const num_attempts) mutable {
   return [](auto const &command) {
      concurrency_collision last_encounter;
      for (std::size_t i = 0; i < num_attempts; ++i) {
         try {
            std::invoke(command_handler, command);
         }
         catch (concurrency_collision const &e) {
            apply_back_pressure(i + 1);
            last_encounter = e;
         }
      }
      throw last_encounter;
   };
};

inline constexpr auto retry_command_handler = [](auto command_handler, auto apply_back_pressure, auto on_command_failure, std::size_t const num_attempts) mutable {
   return [command_handler=std::move(command_handler), apply_back_pressure=std::move(apply_back_pressure), on_command_failure=std::move(on_command_failure), num_attempts](auto const &command) mutable {
      std::exception_ptr last_encounter = nullptr;
      for (std::size_t i = 0; i < num_attempts; ++i) {
         try {
            return std::invoke(command_handler, command);
         }
         catch (...) {
            apply_back_pressure(i + 1);
            last_encounter = nullptr;
         }
      }
      std::rethrow_exception(last_encounter);
   };
};

inline constexpr auto ignore_commit = [](concepts::commit auto const &) noexcept {};

inline constexpr auto check_for_error = []<concepts::commit Commit>(Commit const &c) {
   std::optional const error{c.error()};
   if (error) {
      if constexpr (std::is_convertible_v<decltype(*error), std::exception_ptr>) {
         std::rethrow_exception(*error);
      }
      else if constexpr (std::is_convertible_v<decltype(*error), std::error_code const &>) {
         throw std::system_error{*error, "Commit failed"};
      }
      else if constexpr (std::is_convertible_v<decltype(*error), std::exception const &>) {
         throw *error;
      }
   }
};

inline constexpr entity_command_handler = [](auto command_dispatcher, auto entity_store, auto on_commit) mutable {
   return [command_dispatcher=std::move(command_dispatcher), entity_store=std::move(entity_store), on_commit=std::move(on_commit)](auto const &command) mutable {
      decltype(auto) entity{get_entity(entity_store, entity_id(command))};
      std::invoke(command_dispatcher, get_reference(entity), command);
      on_commit(put_entity(entity_store, std::move(entity)));
   };
};

}