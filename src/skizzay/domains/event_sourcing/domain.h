#pragma once

#include "skizzay/domains/event_sourcing/commit_header.h"
#include "skizzay/domains/event_sourcing/traits.h"

namespace skizzay::domains::event_sourcing {

template<class StreamIdType,
         class StreamVersionIntegralType,
         class CommitIdType,
         class CommitSequenceIntegralType,
         class TimestampType>
struct basic_event_source {
    static_assert(utilz::equivalent_v<StreamIdType>);
    static_assert(std::conjunction_v<std::is_integral<StreamVersionIntegralType>, std::is_unsigned_v<StreamVersionIntegralType>>);
    static_assert(utilz::equivalent_v<CommitIdType>);
    static_assert(std::conjunction_v<std::is_integral<CommitSequenceIntegralType>, std::is_unsigned_v<CommitSequenceIntegralType>>);
    static_assert(utilz::is_time_point_v<TimestampType>);

    using stream_id_type = StreamIdType;
    using stream_version_type = sequence<struct stream_version_type_tag, StreamVersionIntegralType>;
    using commit_id_type = CommitIdType;
    using commit_sequence_type = sequence<struct commit_sequence_type_tag, CommitSequenceIntegralType>;
    using commit_header_type = basic_commit_header<stream_id_type, commit_id_type, commit_version_type, timestamp_type>;
};

}