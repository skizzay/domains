#pragma once

#include "skizzay/domains/event_sourcing/commit_sequence.h"
#include "skizzay/domains/event_sourcing/range_holder.h"
#include "skizzay/domains/event_sourcing/result_holder.h"
#include "skizzay/domains/event_sourcing/stream_version.h"
#include <skizzay/utilz/traits.h>
#include <skizzay/utilz/detected.h>
#include <chrono>
#include <functional>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

enum class commit_result;

namespace details_ {

template <class T>
using typename_stream_id_type = typename T::stream_id_type;

template <class T>
using typename_commit_id_type = typename T::commit_id_type;

template <class T>
using typename_timestamp_type = typename T::timestamp_type;

template <class T>
using commit_store_get_commits_t = decltype(std::declval<T const>().get_commits(
    std::declval<typename_stream_id_type<T> const &>(), std::uint32_t{}, std::uint32_t{}));

#if 0
template <class T>
using commit_store_put_t = decltype(std::declval<T>().put(
    std::declval<
        basic_commit_attempt<typename_stream_id_type<T>, typename_commit_id_type<T>,
                             typename_timestamp_type<T>, commit_store_get_commits_t<T>> const &>()));
#endif

template <class T>
using stream_id_t = decltype(std::declval<T const>().stream_id());

template <class T>
using stream_sequence_t = decltype(std::declval<T const>().stream_sequence());

template <class T>
using commit_sequence_t = decltype(std::declval<T const>().commit_sequence());

template <class T>
using clear_changes_t = decltype(std::declval<T>().clear_changes());

template <class T>
using commited_events_t = decltype(std::declval<T const>().commited_events());

template <class T>
using uncommited_events_t = decltype(std::declval<T const>().uncommited_events());
}

template<class T>
using is_event_stream = std::conjunction<
    utilz::is_detected<details_::typename_stream_id_type, T>,
    utilz::is_detected<details_::stream_id_t, T>,
    std::is_same<
          utilz::remove_cref_t<utilz::detected_t<details_::stream_id_t, T>>,
          utilz::detected_t<details_::typename_stream_id_type, T>
    >,
    utilz::is_template<utilz::detected_t<details_::stream_sequence_t, T>, basic_stream_version>,
    utilz::is_template<utilz::detected_t<details_::commit_sequence_t, T>, basic_commit_sequence>,
    std::is_void<utilz::detected_t<details_::clear_changes_t, T>>,
    utilz::is_range<utilz::detected_t<details_::commited_events_t, T>>,
    utilz::is_range<utilz::detected_t<details_::uncommited_events_t, T>>
>;
template<class T>
inline constexpr bool is_event_stream_v = is_event_stream<T>::value;

template<class T>
using is_commit_store = std::conjunction<
   utilz::equivalent<utilz::detected_t<details_::typename_stream_id_type, T>>,
   utilz::equivalent<utilz::detected_t<details_::typename_commit_id_type, T>>,
   utilz::is_time_point<details_::typename_timestamp_type<T>>,
   utilz::is_range<utilz::detected_t<details_::commit_store_get_commits_t, T>>
>;
template<class T>
inline constexpr bool is_commit_store_v = is_commit_store<T>::value;

}
