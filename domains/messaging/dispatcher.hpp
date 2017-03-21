#pragma once

#include <domains/messaging/buffer.hpp>
#include <domains/utils/memory_resource.hpp>
#include <domains/utils/parameter_pack.hpp>

#include <experimental/memory_resource>

#include <algorithm>
#include <memory>
#include <vector>

namespace domains {
namespace details_ {
template <class Impl, class T, class = void>
struct message_dispatch_helper {
   static void dispatch(Impl &, T const &) noexcept {
   }
};

template <class Impl, class T>
struct message_dispatch_helper<
    Impl, T, std::void_t<decltype(std::declval<Impl>()(std::declval<T const &>()))>> {
   static auto dispatch(Impl &handle, T const &t) noexcept(noexcept(handle(t)))
       -> decltype(handle(t)) {
      return handle(t);
   }
};

template <class T>
struct message_handler_base {
   virtual void handle(T const &t) noexcept = 0;
};

template <class... T>
struct base_message_handler : message_handler_base<T>... {};

template <class, class, class...>
class message_handler;

template <class Impl, class... Ts>
class message_handler<Impl, parameter_pack<Ts...>> : public base_message_handler<Ts...> {
   Impl impl;

public:
   explicit message_handler(Impl &&impl) noexcept(
       std::is_nothrow_constructible<Impl, decltype(std::forward<Impl>(impl))>::value)
      : impl(std::forward<Impl>(impl)) {
   }

protected:
   inline Impl &handler() noexcept {
      return impl;
   }

   inline Impl const &handler() const noexcept {
      return impl;
   }
};

template <class Impl, class... Messages, class T, class... Ts>
class message_handler<Impl, parameter_pack<Messages...>, T, Ts...>
    : public message_handler<Impl, parameter_pack<Messages...>, Ts...> {
public:
   using message_handler<Impl, parameter_pack<Messages...>, Ts...>::message_handler;

   virtual void handle(T const &t) noexcept final override {
      message_dispatch_helper<Impl, T>::dispatch(this->handler(), t);
   }
};
}

template <class...>
struct single_dispatcher;
template <class...>
struct multi_dispatcher;

template <class T>
struct single_dispatcher<T> : T {
   using T::operator();

   explicit single_dispatcher(T &&t) noexcept(
       std::is_nothrow_constructible<T, decltype(std::forward<T>(t))>::value)
      : T(std::forward<T>(t)) {
   }
};

template <class T, class... Ts>
struct single_dispatcher<T, Ts...> : T, single_dispatcher<Ts...> {
   using T::operator();
   using single_dispatcher<Ts...>::operator();

   explicit single_dispatcher(T &&t, Ts &&... ts) noexcept(
       std::is_nothrow_constructible<T, decltype(std::forward<T>(t))>::value
           &&std::is_nothrow_constructible<single_dispatcher<Ts...>,
                                           decltype(std::forward<Ts>(ts))...>::value)
      : T(std::forward<T>(t)), single_dispatcher<Ts...>(std::forward<Ts>(ts)...) {
   }
};

template <class... Ts>
single_dispatcher<Ts...> make_single_dispatcher(Ts &&... ts) noexcept(
    std::is_nothrow_constructible<single_dispatcher<Ts...>,
                                  decltype(std::forward<Ts>(ts))...>::value) {
   return single_dispatcher<Ts...>(std::forward<Ts>(ts)...);
}

template <class... Ts>
class multi_dispatcher final {
   using handler_type = details_::base_message_handler<Ts...>;
   class handler_deleter final {
      std::size_t size;
      std::size_t align;
      std::experimental::pmr::memory_resource *resource;

   public:
      handler_deleter(std::size_t s, std::size_t a,
                      std::experimental::pmr::memory_resource *r) noexcept : size{s},
                                                                             align{a},
                                                                             resource{r} {
      }

      void operator()(handler_type *const handler) noexcept {
         handler->~handler_type();
         resource->deallocate(handler, size, align);
      }
   };
   using handler_ptr = std::unique_ptr<handler_type, handler_deleter>;

   std::vector<handler_ptr, std::experimental::pmr::polymorphic_allocator<handler_ptr>> handlers;

   std::experimental::pmr::memory_resource *resource() const {
      return handlers.get_allocator().resource();
   }

public:
   explicit multi_dispatcher(std::experimental::pmr::memory_resource *resource =
                                 std::experimental::pmr::get_default_resource()) noexcept
       : handlers(resource) {
   }

   template <class F>
   void add_handler(F &&f) {
      using Impl = details_::message_handler<F, parameter_pack<Ts...>, Ts...>;

      Impl *const handler = static_cast<Impl *>(resource()->allocate(sizeof(Impl), alignof(Impl)));
      new (handler) Impl(std::forward<F>(f));
      handlers.emplace_back(handler, handler_deleter{sizeof(Impl), alignof(Impl), resource()});
   }

   template <class... Fs>
   void add_handlers(Fs &&... fs) {
      add_handler(make_single_dispatcher(std::forward<Fs>(fs)...));
   }

   template <class U>
   friend std::enable_if_t<in<U, parameter_pack<Ts...>>::value>
   dispatch(multi_dispatcher<Ts...> &dispatcher, U const &u) noexcept {
      for (auto &handler : dispatcher.handlers) {
         static_cast<details_::message_handler_base<U> *>(handler.get())->handle(u);
      }
   }
};

template <class F, class T>
auto dispatch(F &f,
              T const &t) noexcept(noexcept(details_::message_dispatch_helper<F, T>::dispatch(f,
                                                                                              t)))
    -> decltype(details_::message_dispatch_helper<F, T>::dispatch(f, t)) {
   return details_::message_dispatch_helper<F, T>::dispatch(f, t);
}
}
