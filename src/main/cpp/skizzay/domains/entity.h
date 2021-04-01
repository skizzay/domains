#pragma once

#include <skizzay/domains/tag_dispatch.h>
#include <skizzay/domains/concepts.h>
#include <functional>

namespace skizzay::domains {

inline namespace entity_id_details_ {
inline constexpr struct entity_id_function_ final {
   template <typename E>
   requires tag_invocable<entity_id_function_, E const &> &&
      concepts::identifier<
         tag_invoke_result_t<entity_id_function_, E const &>>
   constexpr auto
      operator()(E const &e) const
      noexcept(nothrow_tag_invocable<entity_id_function_, E const &>)
         -> tag_invoke_result_t<entity_id_function_, E const &> {
      return tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires requires(E const &e) {
      { e.entity_id() }
      -> concepts::identifier;
      requires !tag_invocable<entity_id_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.entity_id()))
      -> decltype(e.entity_id()) {
      return e.entity_id();
   }

   template <typename E>
   requires requires(E const &e) {
      { *e };
      requires std::invocable<entity_id_function_, decltype(*e)>;
      requires concepts::identifier<std::invoke_result_t<entity_id_function_, decltype(*e)>>;
      requires !tag_invocable<entity_id_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const
      noexcept(std::is_nothrow_invocable_v<entity_id_function_, decltype(*e)>)
         -> std::invoke_result_t<entity_id_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }
} entity_id = {};

namespace entity_id_impl_details_ {
template<typename T>
struct entity_id_type_impl {
   using type = decltype(entity_id(std::declval<T>()));
};
}

}

template<typename T>
using entity_id_t = typename entity_id_impl_details_::entity_id_type_impl<T>::type;


inline namespace entity_version_details_ {
inline constexpr struct entity_version_function_ final {
   template <typename E>
   requires tag_invocable<entity_version_function_, E const &> &&
      concepts::sequenced<
         tag_invoke_result_t<entity_version_function_, E const &>>
   constexpr auto
      operator()(E const &e) const
      noexcept(nothrow_tag_invocable<entity_version_function_, E const &>)
         -> tag_invoke_result_t<entity_version_function_, E const &> {
      return tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires requires(E const &e) {
      { e.entity_version() }
      -> concepts::sequenced;
      requires !tag_invocable<entity_version_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.entity_version()))
      -> decltype(e.entity_version()) {
      return e.entity_version();
   }

   template <typename E>
   requires requires(E const &e) {
      { *e };
      requires std::invocable<entity_version_function_, decltype(*e)>;
      requires concepts::sequenced<std::invoke_result_t<entity_version_function_, decltype(*e)>>;
      requires !tag_invocable<entity_version_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const
      noexcept(std::is_nothrow_invocable_v<entity_version_function_, decltype(*e)>)
         -> std::invoke_result_t<entity_version_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }
} entity_version = {};

namespace entity_version_impl_details_ {
template<typename T>
struct entity_version_type_impl {
   using type = decltype(entity_version(std::declval<T>()));
};
}

}

template<typename T>
using entity_version_t = typename entity_version_impl_details_::entity_version_type_impl<T>::type;

namespace concepts {

template<typename T>
concept entity = requires(T const &tc, T &t) {
   { entity_id(tc) } -> concepts::identifier;
   { entity_version(tc) } -> concepts::sequenced;
};

}

}