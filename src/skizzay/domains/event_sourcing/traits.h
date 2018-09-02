#pragma once

#include "skizzay/domains/event_sourcing/range_holder.h"
#include "skizzay/domains/event_sourcing/result_holder.h"
#include <skizzay/utilz/traits.h>
#include <skizzay/utilz/detected.h>
#include <chrono>
#include <functional>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

enum class commit_result;

template<class, class> class event_header;

template<class StreamIdType, class TimestampType, class ...TagsAndEventData>
using basic_event = utilz::dto<utilz::tag<struct domains_event_tag>, event_header<StreamIdType, TimestampType>, TagsAndEventData...>;

template<class ...TagsAndEventData>
using event = basic_event<std::uint64_t, std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>, TagsAndEventData...>;

template<class, class, class> class basic_commit_header;

template<class, class, class, class> struct basic_commit;

template<class EventRange>
using commit = basic_commit<std::uint64_t, std::uint64_t, std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>, EventRange>;

template<class, class, class, class> struct basic_commit_attempt;

template<class EventRange>
using commit_attempt = basic_commit_attempt<std::uint64_t, std::uint64_t, std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>, EventRange>;

template<class, class> class basic_snapshot_header;

template<class StreamIdType, class TimestampType, class MementoType>
using basic_snapshot = utilz::dto<utilz::tag<struct snapshot_tag>, basic_snapshot_header<StreamIdType, TimestampType>, MementoType>;

template<class MementoType>
using snapshot = basic_snapshot<std::uint64_t, std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>, MementoType>;

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

template <class T>
using commit_store_put_t = decltype(std::declval<T>().put(
    std::declval<
        basic_commit_attempt<typename_stream_id_type<T>, typename_commit_id_type<T>,
                             typename_timestamp_type<T>, commit_store_get_commits_t<T>> const &>()));

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
using is_event = utilz::is_template<T, basic_event>;

template<class T>
using is_event_stream = std::conjunction<
    utilz::is_detected<details_::typename_stream_id_type, T>,
    utilz::is_detected<details_::stream_id_t, T>,
    std::is_same<
          utilz::remove_cref_t<utilz::detected_t<details_::stream_id_t, T>>,
          utilz::detected_t<details_::typename_stream_id_type, T>
    >,
    std::is_same<std::uint32_t, utilz::detected_t<details_::stream_sequence_t, T>>,
    std::is_same<std::uint32_t, utilz::detected_t<details_::commit_sequence_t, T>>,
    std::is_void<utilz::detected_t<details_::clear_changes_t, T>>,
    utilz::is_range<utilz::detected_t<details_::commited_events_t, T>>,
    utilz::is_range<utilz::detected_t<details_::uncommited_events_t, T>>
>;
template<class T>
inline constexpr bool is_event_stream_v = is_event_stream<T>::value;

template<class T>
using is_basic_commit = utilz::is_template<T, basic_commit>;

template<class T>
inline constexpr bool is_basic_commit_v = is_basic_commit<T>::value;

template<class T>
using is_commit = utilz::is_template<T, commit>;
template<class T>
inline constexpr bool is_commit_v = is_commit<T>::value;

template<class T>
using is_commit_store = std::conjunction<
   utilz::equivalent<utilz::detected_t<details_::typename_stream_id_type, T>>,
   utilz::equivalent<utilz::detected_t<details_::typename_commit_id_type, T>>,
   utilz::is_time_point<details_::typename_timestamp_type<T>>,
   utilz::is_range<utilz::detected_t<details_::commit_store_get_commits_t, T>>
>;
template<class T>
inline constexpr bool is_commit_store_v = is_commit_store<T>::value;

template<class T>
using is_basic_snapshot = utilz::is_template<T, basic_snapshot>;
template<class T>
inline constexpr bool is_basic_snapshot_v = is_basic_snapshot<T>::value;

template<class T>
using is_snapshot = utilz::is_template<T, snapshot>;
template<class T>
inline constexpr bool is_snapshot_v = is_snapshot<T>::value;

}
