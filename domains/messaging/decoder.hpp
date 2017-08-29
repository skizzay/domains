#pragma once

#include <domains/messaging/dispatcher.hpp>
#include <domains/messaging/router.hpp>
#include <domains/messaging/translator.hpp>

namespace domains {
struct null_domain_dispatcher_t final {
   template <class T>
   constexpr std::error_code operator()(T const &) const noexcept {
      return {};
   }
};
constexpr null_domain_dispatcher_t null_domain_dispatcher{};

template <class Router, class Translator>
class decoder {
   std::decay_t<Router> route;
   std::decay_t<Translator> translator;

public:
   decoder() noexcept = default;
   decoder(Router r, Translator t) : route(std::move(r)), translator(std::move(t)) {
   }

   template <class DomainDispatcher, class EncodedType>
   std::error_code operator()(DomainDispatcher &&domain_dispatcher, EncodedType &&encoded_value) {
      return route(std::forward<EncodedType>(encoded_value),
                   translator.dispatch_to(domain_dispatcher));
   }
};

using null_decoder_t = decoder<null_router_t, null_translator_t>;
}
