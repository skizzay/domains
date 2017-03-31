#pragma once

#include <domains/messaging/dispatcher.hpp>
#include <domains/utils/function_traits.hpp>
#include <domains/utils/type_provider.hpp>

namespace domains {
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

template <class Router, class DecodeDispatcher, class TypeProvider>
class decoder {
   std::decay_t<Router> route;
   std::decay_t<DecodeDispatcher> decode;
   std::decay_t<TypeProvider> type_provider;

   template <class DomainDispatcher>
   class impl final {
      decoder &d;
      DomainDispatcher &&dispatch;

   public:
      explicit impl(decoder &d_, DomainDispatcher &&dd_) noexcept : d{d_}, dispatch{dd_} {
      }

      template <class DecodedType, class EncodedType>
      std::error_code decode_and_dispatch(EncodedType &&encoded_value) {
         auto decoded_object = d.type_provider.template provide<DecodedType>();
         std::error_code error =
             d.decode(std::forward<EncodedType>(encoded_value), *decoded_object);
         if (!error) {
            error = dispatch(*decoded_object);
         }
         return error;
      }
   };

public:
   decoder() noexcept = default;
   decoder(Router &&r, DecodeDispatcher &&dd, TypeProvider &&tp)
      : route(std::move(r)), decode(std::move(dd)), type_provider(std::move(tp)) {
   }

   template <class DomainDispatcher, class EncodedType>
   std::error_code operator()(DomainDispatcher &&domain_dispatcher, EncodedType &&encoded_value) {
      return route(
          std::forward<EncodedType>(encoded_value),
          impl<DomainDispatcher>(*this, std::forward<DomainDispatcher>(domain_dispatcher)));
   }
};

template <class Router, class DecodeDispatcher, class TypeProvider>
decoder<std::decay_t<Router>, std::decay_t<DecodeDispatcher>, std::decay_t<TypeProvider>>
make_decoder(Router router, DecodeDispatcher decode_dispatcher, TypeProvider type_provider) {
   return decoder<std::decay_t<Router>, std::decay_t<DecodeDispatcher>, std::decay_t<TypeProvider>>(
       std::move(router), std::move(decode_dispatcher), std::move(type_provider));
}

using null_decoder_t = decoder<null_router_t, null_decode_dispatcher_t, null_type_provider_t>;
}
