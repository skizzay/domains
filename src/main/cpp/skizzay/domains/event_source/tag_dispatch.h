#pragma once

#include <type_traits>

// NOTE:  This is being ripped from
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf Personally, I think C++ needs
// a better mechanism than tag dispatching

namespace skizzay::domains::event_source {

inline namespace tag_invoke_details_ {

inline constexpr struct {
   template <typename Tag, typename... Args>
   requires requires(Tag tag, Args &&...args) {
      tag_invoke((Tag &&) tag, (Args &&) args...);
   }
   constexpr auto operator()(Tag tag, Args &&...args) const
      noexcept(noexcept(tag_invoke((Tag &&) tag, (Args &&) args...)))
         -> decltype(tag_invoke((Tag &&) tag, (Args &&) args...)) {
      return tag_invoke((Tag &&) tag, (Args &&) args...);
   }
} tag_invoke = {};

} // namespace tag_invoke_details_

template <typename Tag, typename... Args>
concept tag_invocable = requires(Tag tag, Args &&...args) {
   skizzay::domains::event_source::tag_invoke((Tag &&) tag, (Args &&) args...);
};

template <typename Tag, typename... Args>
concept nothrow_tag_invocable = tag_invocable<Tag, Args...> &&requires(Tag tag, Args &&...args) {
   { skizzay::domains::event_source::tag_invoke((Tag &&) tag, (Args &&) args...) }
   noexcept;
};

template <typename Tag, typename... Args>
using tag_invoke_result =
   std::invoke_result<decltype(skizzay::domains::event_source::tag_invoke), Tag, Args...>;

template <typename Tag, typename... Args>
using tag_invoke_result_t = typename tag_invoke_result<Tag, Args...>::type;

template <auto &Tag>
using tag_t = std::decay_t<decltype(Tag)>;

} // namespace skizzay::domains::event_source