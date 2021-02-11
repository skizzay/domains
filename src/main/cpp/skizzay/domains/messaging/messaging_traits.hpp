#pragma once

#include <domains/utils/function_traits.h>

namespace domains {
template <class F>
using message_from_argument = std::decay_t<typename function_traits<F>::template argument<0>::type>;
template <class F>
using message_from_result = std::decay_t<typename function_traits<F>::result_type>;
}
