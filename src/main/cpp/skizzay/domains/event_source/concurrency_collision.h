#pragma once

#include <stdexcept>

namespace skizzay::domains::event_source {

struct concurrency_collision : std::runtime_error {
   using runtime_error::runtime_error;
};

}