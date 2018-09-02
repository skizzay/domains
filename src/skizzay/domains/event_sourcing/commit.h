#pragma once

#include "skizzay/domains/event_sourcing/commit_header.h"
#include "skizzay/domains/event_sourcing/commit_result.h"
#include "skizzay/domains/event_sourcing/result_holder.h"
#include <skizzay/utilz/dto.h>

namespace skizzay::domains::event_sourcing {

template<class StreamIdType, class CommitIdType, class TimestampType, class EventRange>
struct basic_commit : utilz::dto<utilz::tag<struct commit_tag>, basic_commit_header<StreamIdType, CommitIdType, TimestampType>, EventRange, result_holder<commit_result>> {
#if 1
   using utilz::dto<utilz::tag<struct commit_tag>, basic_commit_header<StreamIdType, CommitIdType, TimestampType>, EventRange, result_holder<commit_result>>::dto;
#elif 0
   constexpr basic_commit(basic_commit_header<StreamIdType, CommitIdType, TimestampType> const &header, EventRange events, commit_result const cr) noexcept :
      utilz::dto<utilz::tag<struct commit_tag>, basic_commit_header<StreamIdType, CommitIdType, TimestampType>, EventRange, result_holder<commit_result>>{header, std::move(events), cr}
   {
   }
#endif
};

#if 1
template<class StreamIdType, class CommitIdType, class TimestampType, class EventRange>
basic_commit(basic_commit_header<StreamIdType, CommitIdType, TimestampType> const &, EventRange, result_holder<commit_result> const) -> basic_commit<StreamIdType, CommitIdType, TimestampType, EventRange>;
#endif

}
