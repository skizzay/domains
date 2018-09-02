#pragma once

#include "skizzay/domains/event_sourcing/commit_header.h"
#include "skizzay/domains/event_sourcing/commit_result.h"
#include "skizzay/domains/event_sourcing/result_holder.h"
#include <skizzay/utilz/dto.h>

namespace skizzay::domains::event_sourcing {

template<class StreamIdType, class CommitIdType, class TimestampType, class EventRange>
struct basic_commit_attempt : utilz::dto<utilz::tag<struct commit_attempt_tag>, basic_commit_header<StreamIdType, CommitIdType, TimestampType>, EventRange> {
#if 1
   using utilz::dto<utilz::tag<struct commit_attempt_tag>, basic_commit_header<StreamIdType, CommitIdType, TimestampType>, EventRange>::dto;
#elif 0
   constexpr basic_commit_attempt(basic_commit_header<StreamIdType, CommitIdType, TimestampType> const &header, EventRange events) noexcept :
      utilz::dto<utilz::tag<struct commit_attempt_tag>, basic_commit_header<StreamIdType, CommitIdType, TimestampType>, EventRange>{header, std::move(events)}
   {
   }
#endif
};

#if 1
template<class StreamIdType, class CommitIdType, class TimestampType, class EventRange>
basic_commit_attempt(basic_commit_header<StreamIdType, CommitIdType, TimestampType> const &, EventRange) -> basic_commit_attempt<StreamIdType, CommitIdType, TimestampType, EventRange>;
#endif

}
