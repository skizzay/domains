#pragma once

#include "skizzay/domains/event_sourcing/commit.h"
#include "skizzay/domains/event_sourcing/commit_attempt.h"
#include "skizzay/domains/event_sourcing/commit_result.h"
#include "skizzay/domains/event_sourcing/traits.h"
#include <skizzay/utilz/traits.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>

namespace skizzay::domains::event_sourcing {

template<class T, class Pipeline>
class commit_store {
   static_assert(is_commit_store_v<utilz::strip_reference_wrapper_t<T>>, "T must be a commit store");

   class storage : Pipeline {
      T t;

   public:
      constexpr storage(T &&t, Pipeline &&hooks) noexcept :
         Pipeline{std::forward<Pipeline>(hooks)},
         t{std::forward<T>(t)}
      {
      }

      constexpr Pipeline &pipeline() noexcept {
         return *this;
      }

      constexpr utilz::strip_reference_wrapper_t<T> &store() noexcept {
         return utilz::strip_reference_wrapper<T>::apply(t);
      }

      constexpr utilz::strip_reference_wrapper_t<T> const &store() const noexcept {
         return utilz::strip_reference_wrapper<T>::apply(t);
      }
   } storage_;



public:
   using stream_id_type = typename utilz::strip_reference_wrapper_t<T>::stream_id_type;
   using stream_version_type = typename utilz::strip_reference_wrapper_t<T>::stream_version_type;
   using commit_id_type = typename utilz::strip_reference_wrapper_t<T>::commit_id_type;
   using commit_sequence_type = typename utilz::strip_reference_wrapper_t<T>::commit_sequence_type;
   using timestamp_type = typename utilz::strip_reference_wrapper_t<T>::timestamp_type;
   using commit_header_type = basic_commit_header<stream_id_type, stream_version_type, commit_id_type, commit_sequence_type, timestamp_type>;

#if 0
   using commit_range_type =
      decltype(strip(std::declval<std::add_lvalue_reference_t<T>>()).get_commits(std::declval<stream_id_type const>(), std::uint32_t{}, std::uint32_t{}));
   using commit_type = utilz::remove_cref_t<decltype(*std::declval<utilz::begin_t<commit_range_type const>>())>;
#endif

   constexpr commit_store(T t, Pipeline &&hooks) noexcept :
      storage_{std::move(t), std::forward<Pipeline>(hooks)}
   {
   }

   constexpr auto get_commits(
         stream_id_type const &sid,
         stream_version_type const min_sequence={},
         stream_version_type const max_sequence=stream_version_type::max()) const {
      return storage_.store().get_commits(sid, min_sequence, max_sequence);
   }

   template <class EventRange>
   constexpr auto put(basic_commit_attempt<commit_header_type, EventRange> const &attempt) {
      using commit_type = basic_commit<commit_header_type, decltype(get_empty_range())>;
      commit_type const c{
         storage_.pipeline().on_commit_attempt(attempt)
            ? storage_.store().put(attempt)
            : commit_type{attempt, get_empty_range(), commit_result::pipeline_invalidated}
      };
      if (c == commit_result::success) {
         storage_.pipeline().on_commit(c);
      }
      return c;
   }

   constexpr auto get_empty_range() {
      return storage_.store().get_empty_range();
   }
};

template<class T, class U>
commit_store(T &&, U &&) -> commit_store<T, U>;

template<class T, class U>
commit_store(T const &, U &&) -> commit_store<T, U>;

}
