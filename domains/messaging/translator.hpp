#pragma once

#include <domains/utils/type_provider.hpp>
#include <domains/utils/type_traits.hpp>

#include <system_error>

namespace domains {
class dynamic_casting_translator {
   template <class DomainDispatcher>
   class impl final {
      DomainDispatcher &dispatch;

   public:
      explicit impl(DomainDispatcher &d) noexcept : dispatch{d} {
      }

      template <class DecodedType, class EncodedType>
      std::enable_if_t<is_base_of_v<std::decay_t<EncodedType>, std::decay_t<DecodedType>>,
                       std::error_code>
      decode_and_dispatch(EncodedType &&encoded_value) {
         return dispatch(dynamic_cast<DecodedType const &>(encoded_value));
      }
   };

public:
   template <class DomainDispatcher>
   impl<std::decay_t<DomainDispatcher>> dispatch_to(DomainDispatcher &domain_dispatcher) noexcept {
      return impl<std::decay_t<DomainDispatcher>>{domain_dispatcher};
   }
};

class static_casting_translator {
   template <class DomainDispatcher>
   class impl final {
      DomainDispatcher &dispatch;

   public:
      explicit impl(DomainDispatcher &d) noexcept : dispatch{d} {
      }

      template <class DecodedType, class EncodedType>
      std::enable_if_t<is_base_of_v<std::decay_t<EncodedType>, std::decay_t<DecodedType>>,
                       std::error_code>
      decode_and_dispatch(EncodedType &&encoded_value) {
         return dispatch(static_cast<DecodedType const &>(encoded_value));
      }
   };

public:
   template <class DomainDispatcher>
   impl<std::decay_t<DomainDispatcher>> dispatch_to(DomainDispatcher &domain_dispatcher) noexcept {
      return impl<std::decay_t<DomainDispatcher>>{domain_dispatcher};
   }
};

struct null_decode_dispatcher_t final {
   template <class T, class U>
   std::error_code operator()(T const &, U &&) const noexcept {
      return {};
   }
};
constexpr null_decode_dispatcher_t null_decode_dispatcher{};

template <class DecodeDispatcher, class TypeProvider>
class parsing_translator {
   std::decay_t<DecodeDispatcher> decode;
   std::decay_t<TypeProvider> type_provider;

   template <class DomainDispatcher>
   class impl final {
      parsing_translator &self;
      DomainDispatcher &dispatch;

   public:
      explicit impl(parsing_translator &pt, DomainDispatcher &d) noexcept : self{pt}, dispatch{d} {
      }

      template <class DecodedType, class EncodedType>
      std::error_code decode_and_dispatch(EncodedType &&encoded_value) {
         auto decoded_object = self.type_provider.template provide<DecodedType>();
         std::error_code const error =
             self.decode(std::forward<EncodedType>(encoded_value), *decoded_object);
         if (!error) {
            return dispatch(*decoded_object);
         }
         return error;
      }
   };

public:
   parsing_translator() noexcept = default;
   explicit parsing_translator(DecodeDispatcher &&dd, TypeProvider &&tp) noexcept(
       is_nothrow_move_constructible_v<DecodeDispatcher>
           &&is_nothrow_move_constructible_v<TypeProvider>)
      : decode{std::move(dd)}, type_provider{std::move(tp)} {
   }

   template <class DomainDispatcher>
   impl<std::decay_t<DomainDispatcher>> dispatch_to(DomainDispatcher &dispatcher) noexcept {
      return impl<std::decay_t<DomainDispatcher>>{*this, dispatcher};
   }
};

template <class DecodeDispatcher, class TypeProvider>
parsing_translator<std::decay_t<DecodeDispatcher>, std::decay_t<TypeProvider>>
make_parsing_translator(DecodeDispatcher decode_dispatcher, TypeProvider type_provider) {
   return parsing_translator<std::decay_t<DecodeDispatcher>, std::decay_t<TypeProvider>>{
       std::move(decode_dispatcher), std::move(type_provider)};
}

using null_translator_t = parsing_translator<null_decode_dispatcher_t, null_type_provider_t>;
constexpr null_translator_t null_translator;
}
