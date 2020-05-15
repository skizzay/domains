#pragma once

#include "skizzay/domains/event_sourcing/commit_header.h"
#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

#if 0
template<class CommitHeaderType, class EventRange>
using basic_commit_attempt = utilz::dto<
      utilz::tag<struct basic_commit_attempt_tag>,
      std::enable_if_t<utilz::is_template_v<CommitHeaderType, basic_commit_header>, CommitHeaderType>,
      EventRange>;
#elif 1

template<class CommitHeaderType, class EventRange>
struct basic_commit_attempt : utilz::dto<
      utilz::tag<struct basic_commit_attempt_tag>,
      std::enable_if_t<utilz::is_template_v<CommitHeaderType, basic_commit_header>, CommitHeaderType>,
      EventRange> {
   using utilz::dto<
      utilz::tag<struct basic_commit_attempt_tag>,
      std::enable_if_t<utilz::is_template_v<CommitHeaderType, basic_commit_header>, CommitHeaderType>,
      EventRange>::dto;
};

template<class CHT, class ER>
basic_commit_attempt(CHT, ER) -> basic_commit_attempt<CHT, ER>;
#endif

}