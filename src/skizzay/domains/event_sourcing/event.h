#pragma once

#include "skizzay/domains/event_sourcing/event_header.h"
#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<concepts::event_header EventHeader, class ...TagsAndEventData>
using basic_event = utilz::dto<
        utilz::tag<struct basic_event_tag>,
        EventHeader,
        TagsAndEventData...>;

namespace concepts {
namespace details_ {
   template<class T, class=void>
   struct dto_based : std::false_type {
   };

   template<class T>
   inline constexpr bool is_dto_based = std::disjunction_v<utilz::is_dto<T>, utilz::inherits_from_template<T, utilz::dto>>;

   template<class T>
   inline constexpr bool has_basic_event_tag = (0 < utilz::length_v<typename T::tag_types>) && (std::is_same_v<utilz::at_t<typename T::tag_types, 0>, utilz::tag<skizzay::domains::event_sourcing::basic_event_tag>>);

   template<class T>
   inline constexpr bool has_event_header = (0 < utilz::length_v<typename T::data_types>) && (event_header<utilz::at_t<typename T::data_types, 0>>);

   template<class T>
   struct dto_based<T, std::enable_if_t<is_dto_based<T>>> : std::bool_constant<has_basic_event_tag<T> && has_event_header<T>> {
   };

   template<class T>
   struct is_event_concept_impl : dto_based<T> {
   };

   template<class... Ts>
   struct is_event_concept_impl<std::variant<Ts...>> : std::conjunction<is_event_concept_impl<Ts>...> {
   };
}

#if 1
   template<class T>
   concept event = details_::is_event_concept_impl<T>::value;
#else
        template<class T>
        concept event = (utilz::is_template_v<T, utilz::dto> || utilz::inherits_from_template_v<T, utilz::dto>)
                && requires {
                0 < utilz::length_v<typename T::tag_types>;
                std::is_same_v<utilz::at_t<typename T::tag_types, 0>, utilz::tag<skizzay::domains::event_sourcing::basic_event_tag>>;
                0 < utilz::length_v<typename T::data_types>;
                requires event_header<utilz::at_t<typename T::data_types, 0>>;
        };
#endif

        template<class T>
        concept event_range = std::ranges::range<T> && requires {
                typename T::value_type;
                requires event<typename T::value_type>;
        };

        template<class T>
        concept event_iterator = std::input_iterator<T> && event<std::iter_value_t<T>>;
}

}