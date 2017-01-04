#pragma once

#include <domains/utils/function_traits.h>
//#include "domains/utils/type_id_generator.h"

namespace domains {

// typedef type_id_generator::type_id message_type_id;

template <class F>
using message_from_argument = std::decay_t<typename function_traits<F>::template argument<0>::type>;
template <class F>
using message_from_result = std::decay_t<typename function_traits<F>::result_type>;
}
