#pragma once

#include "skizzay/domains/event_sourcing/traits.h"
#include <skizzay/utilz/detected.h>
#include <utility>

namespace skizzay::domains::event_sourcing {

namespace details_ {
template<class Hook, class CommitAttempt>
using on_commit_attempt_t = decltype(std::declval<Hook>().on_commit_attempt(std::declval<CommitAttempt const &>()));

template<class Hook, class Commit>
using on_commit_t = decltype(std::declval<Hook>().on_commit(std::declval<Commit const &>()));

template<class Hook, class StreamIdType>
using on_stream_deleted_t = decltype(std::declval<Hook>().on_stream_deleted(std::declval<StreamIdType const &>()));

template<class Hook, class CommitAttempt>
constexpr bool on_commit_attempt(Hook &h, CommitAttempt const &ca) {
   if constexpr (utilz::is_detected_convertible_v<bool, on_commit_attempt_t, Hook, CommitAttempt>) {
      return h.on_commit_attempt(ca);
   }
   else {
      return true;
   }
}

template<class Hook, class Commit>
constexpr void on_commit(Hook &h, Commit const &c) {
   if constexpr (std::is_void_v<utilz::detected_t<on_commit_t, Hook, Commit>>) {
      return h.on_commit(c);
   }
}

template<class Hook, class StreamIdType>
constexpr void on_stream_deleted(Hook &h, StreamIdType const &sid) {
   if constexpr (std::is_void_v<utilz::detected_t<on_stream_deleted_t, Hook, StreamIdType>>) {
      return h.on_stream_deleted(sid);
   }
}

}

template<class ...Hooks>
struct hook : Hooks ... {
   constexpr hook(Hooks &&... hooks)
      noexcept(std::conjunction_v<std::is_nothrow_constructible<Hooks, decltype(std::forward<Hooks>(hooks))>...>)
      : Hooks{std::forward<Hooks>(hooks)}...
   {
   }

   template<class CommitAttempt>
   constexpr bool on_commit_attempt(CommitAttempt const &ca) {
      return (details_::on_commit_attempt(utilz::strip_reference_wrapper<Hooks>::apply(*static_cast<Hooks *>(this)), ca) && ...);
   }

   template<class Commit>
   constexpr void on_commit(Commit const &ca) {
      (details_::on_commit(utilz::strip_reference_wrapper<Hooks>::apply(*static_cast<Hooks *>(this)), ca), ...);
   }

   template<class StreamIdType>
   constexpr void on_stream_deleted(StreamIdType const &sid) {
      (details_::on_stream_deleted(utilz::strip_reference_wrapper<Hooks>::apply(*static_cast<Hooks *>(this)), sid), ...);
   }
};

template<class ...Hooks>
hook(Hooks &&...) -> hook<Hooks...>;

}
