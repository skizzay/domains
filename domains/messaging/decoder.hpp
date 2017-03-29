#pragma once

#include <domains/messaging/dispatcher.hpp>
#include <domains/utils/function_traits.hpp>

namespace domains {
#if 0
namespace details_ {
template <class Switch, class... Translators>
class decoder_impl {
   using decoded_types = std::tuple<argument_of_t<Translators, 1>...>;

   static_assert(is_unique<as_parameter_pack_t<decoded_types>>::value,
                 "Ambiguous translation was found in decoder.");

   Switch swtch;
   decoded_types decoded_objects;
   single_dispatcher<std::decay_t<Translators>...> decode_dispatcher;

   template <class DomainDispatcher, class EncodedType>
   class impl final {
      decoder_impl &d;

      template <class DecodedType>
      std::error_code decode_and_dispatch(DomainDispatcher &&, EncodedType &&,
                                          std::false_type) noexcept {
         return make_error_code(std::errc::not_supported);
      }

      template <class DecodedType>
      std::error_code decode_and_dispatch(DomainDispatcher &&domain_dispatcher,
                                          EncodedType &&encoded_value, std::true_type) {
         std::error_code error =
             d.decode_dispatcher(encoded_value, std::get<DecodedType>(d.decoded_objects));
         if (!error) {
            error = std::forward<DomainDispatcher>(domain_dispatcher)(
                std::get<DecodedType>(d.decoded_objects));
         }
         return error;
      }

   public:
      explicit impl(decoder_impl &_d) noexcept : d{_d} {
      }

      template <class DecodedType>
      std::error_code operator()(DomainDispatcher &&domain_dispatcher,
                                 EncodedType &&encoded_value) {
         constexpr bool is_safe =
             is_dispatchable<single_dispatcher<Translators...>, EncodedType, DecodedType>::value &&
             is_dispatchable<DomainDispatcher, DecodedType>::value;
         return this->decode_and_dispatch<DecodedType>(
             std::forward<DomainDispatcher>(domain_dispatcher), encoded_value,
             std::integral_constant<bool, is_safe>{});
      }
   };

public:
   explicit decoder_impl(Switch &&s, single_dispatcher<Translators...> &&t)
      : swtch{std::forward<Switch>(s)},
        decode_dispatcher{std::forward<single_dispatcher<Translators...>>(t)} {
   }

   template <class DomainDispatcher, class EncodedType>
   std::error_code operator()(DomainDispatcher &&domain_dispatcher, EncodedType &&encoded_value) {
      return swtch(std::forward<DomainDispatcher>(domain_dispatcher),
                   std::forward<EncodedType>(encoded_value),
                   impl<DomainDispatcher, EncodedType>{*this});
   }
};
}

template <class Switch, class... Translators>
struct decoder final : details_::decoder_impl<std::decay_t<Switch>, std::decay_t<Translators>...> {
   using details_::decoder_impl<std::decay_t<Switch>, std::decay_t<Translators>...>::decoder_impl;
   using details_::decoder_impl<std::decay_t<Switch>, std::decay_t<Translators>...>::operator();
};

template <class Switch, class... Translators>
decoder<Switch, Translators...> make_decoder(Switch swtch, Translators &&... translators) noexcept {
   return {std::move(swtch), make_single_dispatcher(std::forward<Translators>(translators)...)};
}
#endif

template <class>
struct decode_type final {};

namespace details_ {
template <class Router, class DecodeDispatcher, class... DecodedTypes>
class decoder_impl {
   Router route;
   std::aligned_union_t<0, DecodedTypes...> decoded_value_holder;
   DecodeDispatcher decode_dispatcher;

   struct destruct_only final {
      template <class T>
      std::enable_if_t<std::is_trivially_destructible<T>::value> operator()(T *const) noexcept {
      }

      template <class T>
      std::enable_if_t<!std::is_trivially_destructible<T>::value> operator()(T *const t) noexcept {
         t->~T();
      }
   };

   template <class DecodedType>
   std::unique_ptr<DecodedType, destruct_only> make_object() noexcept {
      return {new (&decoded_value_holder) DecodedType(), destruct_only{}};
   }

   template <class DomainDispatcher>
   class impl final {
      decoder_impl &d;
      DomainDispatcher &&domain_dispatcher;

   public:
      explicit impl(decoder_impl &d_, DomainDispatcher &&dd_) noexcept : d{d_},
                                                                         domain_dispatcher{dd_} {
      }

      template <class DecodedType, class EncodedType>
      std::error_code operator()(EncodedType &&encoded_value, decode_type<DecodedType> = {}) {
         auto decoded_object = d.make_object<DecodedType>();
         std::error_code error =
             d.decode_dispatcher(std::forward<EncodedType>(encoded_value), *decoded_object);
         if (!error) {
            error = domain_dispatcher(*decoded_object);
         }
         return error;
      }
   };

public:
   explicit decoder_impl(Router &&router, DecodeDispatcher &&decoder)
      : route{std::forward<Router>(router)},
        decode_dispatcher{std::forward<DecodeDispatcher>(decoder)} {
   }

   template <class DomainDispatcher, class EncodedType>
   std::error_code operator()(DomainDispatcher &&domain_dispatcher, EncodedType &&encoded_value) {
      return route(std::forward<EncodedType>(encoded_value),
                   impl<DomainDispatcher>{*this, domain_dispatcher});
   }
};
}

template <class Router, class DecodeDispatcher, class... DecodedTypes>
struct decoder : details_::decoder_impl<std::decay_t<Router>, std::decay_t<DecodeDispatcher>,
                                        std::decay_t<DecodedTypes>...> {
   static_assert(is_unique_v<parameter_pack<std::decay_t<DecodedTypes>>...>,
                 "Decoded types must be not list types more than once.");
   using base_type_ = details_::decoder_impl<std::decay_t<Router>, std::decay_t<DecodeDispatcher>,
                                             std::decay_t<DecodedTypes>...>;
   using base_type_::decoder_impl;
   using base_type_::operator();
};
}
