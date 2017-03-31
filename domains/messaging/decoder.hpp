#pragma once

#include <domains/messaging/dispatcher.hpp>
#include <domains/utils/function_traits.hpp>

namespace domains {
template <class>
struct decode_type final {};

struct null_router_t final {
   template <class T, class U>
   constexpr std::error_code operator()(T const &&, U &&) const noexcept {
      return {};
   }
};
constexpr null_router_t null_router{};

struct null_decode_dispatcher_t final {
   template <class T, class U>
   constexpr std::error_code operator()(T const &&, U &) const noexcept {
      return {};
   }
};
constexpr null_decode_dispatcher_t null_decode_dispatcher{};

struct null_domain_dispatcher_t final {
   template <class T>
   constexpr std::error_code operator()(T const &) const noexcept {
      return {};
   }
};
constexpr null_domain_dispatcher_t null_domain_dispatcher{};

namespace details_ {
template <class Router, class DecodeDispatcher, class... DecodedTypes>
class decoder_impl {
   Router route;
   std::aligned_union_t<0, DecodedTypes...> decoded_value_holder;
   DecodeDispatcher decode_dispatcher;

   struct destruct_only final {
      template <class T>
      void operator()(T *const t) noexcept {
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
   explicit decoder_impl(Router &&router = {}, DecodeDispatcher &&decoder = {})
      : route{std::forward<Router>(router)},
        decode_dispatcher{std::forward<DecodeDispatcher>(decoder)} {
   }

   template <class DomainDispatcher, class EncodedType>
   std::error_code operator()(DomainDispatcher &&domain_dispatcher, EncodedType &&encoded_value) {
      return route(
          std::forward<EncodedType>(encoded_value),
          impl<DomainDispatcher>{*this, std::forward<DomainDispatcher>(domain_dispatcher)});
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

template <class... T>
using null_decoder_t = decoder<null_router_t, null_decode_dispatcher_t, T...>;
}
