#pragma once

#include <domains/event_source/commit.hpp>
#include <domains/event_source/commit_attempt.hpp>

#include <limits>

namespace domains {
template <CommitRange commit_range>
class commit_query_result final {
   std::error_code error_;
   commit_range commits_;

public:
   commit_query_result(std::error_code e, CommitRange c) noexcept : error_{std::move(e)},
                                                                    commits_{std::move(c)} {
   }

   explicit commit_query_result(std::error_code e) noexcept : error_{std::move(e)}, commits_{} {
   }

   explicit commit_query_result(CommitRange c) : error_{}, commits_{std::move(c)} {
   }

   std::error_code error() const noexcept {
      return error_;
   }

   commit_range commits() const noexcept {
      return commits_;
   }
};

auto begin(commit_query_result<CommitRange> const &cqr) noexcept {
   using std::begin;
   return begin(cqr.commits());
}

auto end(commit_query_result<CommitRange> const &cqr) noexcept {
   using std::end;
   return end(cqr.commits());
}

template <typename T>
concept bool EventCommitter = requires(T t) {
   typename T::id_type;
   commit_query_result<CommitRange> get_commits(
       IdType const bucket_id, IdType const stream_id, uint64_t min_version = 0U,
       uint64_t max_version = std::numeric_limits<uint64_t>::max())
       const noexcept; // range of commits
   commit_query_result<CommitRange> get_undispatched_commits() const noexcept;
   commit_query_result<CommitRange> try_put(commit_attempt<IdType> const &attempt);
   std::error_code mark_dispatched(commit<IdType> const &c) noexcept;
};
}
