#pragma once

#include <exception>

namespace skizzay::domains::event_sourcing {

enum class commit_result {
   success,
   no_pending_events,
   concurrency_collision,
   pipeline_invalidated,
   internal_error
};

constexpr char const * to_string(commit_result const cr) noexcept {
   switch (cr) {
      case commit_result::success: return "success";
      case commit_result::no_pending_events: return "no pending events";
      case commit_result::concurrency_collision: return "concurrency collision";
      case commit_result::pipeline_invalidated: return "pipeline invalidated";
      case commit_result::internal_error: return "internal error";
      default: std::terminate();
   }
}

}
