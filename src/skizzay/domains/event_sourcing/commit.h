#pragma once

#include "skizzay/domains/event_sourcing/commit_header.h"
#include "skizzay/domains/event_sourcing/commit_result.h"
#include "skizzay/domains/event_sourcing/result_holder.h"
#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

#if 0
template<class CommitHeaderType, class EventRange>
using basic_commit = utilz::dto<
      utilz::tag<struct basic_commit_tag>,
      std::enable_if_t<utilz::is_template_v<CommitHeaderType, basic_commit_header>, CommitHeaderType>,
      result_holder<commit_result>,
      EventRange>;
#elif 1

template<class CommitHeaderType, class EventRange>
struct basic_commit : utilz::dto<
      utilz::tag<struct basic_commit_tag>,
      std::enable_if_t<utilz::is_template_v<CommitHeaderType, basic_commit_header>, CommitHeaderType>,
      result_holder<commit_result>,
      EventRange> {
   using utilz::dto<
      utilz::tag<struct basic_commit_tag>,
      std::enable_if_t<utilz::is_template_v<CommitHeaderType, basic_commit_header>, CommitHeaderType>,
      result_holder<commit_result>,
      EventRange>::dto;
};

template<class CHT, class ER>
basic_commit(CHT, ER) -> basic_commit<CHT, ER>;
#endif

template<class T>
using is_basic_commit = utilz::is_template<T, basic_commit>;

template<class T>
inline constexpr bool is_basic_commit_v = is_basic_commit<T>::value;

}
