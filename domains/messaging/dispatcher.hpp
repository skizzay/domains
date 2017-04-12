#pragma once

#include <domains/utils/parameter_pack.hpp>

#include <experimental/memory_resource>
#include <experimental/vector>

#include <algorithm>
#include <memory>

namespace domains {
namespace details_ {
template <class, class = void, class = void>
struct is_callable_helper : std::false_type {};

template <class T>
struct is_callable_helper<T, void, std::void_t<std::result_of_t<T>>> : std::true_type {};

template <class T, class R>
struct is_callable_helper<T, R, std::void_t<std::result_of_t<T>>>
    : std::is_convertible<std::result_of_t<T>, R> {};
}

template <class, class = void>
struct is_callable;

template <class T, class R>
struct is_callable : details_::is_callable_helper<T, R> {};

template <class T, class R = void>
constexpr bool is_callable_v = is_callable<T, R>::value;

namespace details_ {
template <class F>
class dispatch_holder : public F {
public:
   using F::operator();

   explicit dispatch_holder(F &&f) : F{std::forward<F>(f)} {
   }
};

template <class R, class... Args>
class dispatch_holder<R (*)(Args...)> {
   using F = R (*)(Args...);
   F f;

public:
   explicit dispatch_holder(F dispatcher) noexcept : f{dispatcher} {
   }

   R operator()(Args... args) const noexcept(noexcept(f(args...))) {
      return f(args...);
   }
};

template <class...>
struct single_dispatcher_impl;

template <class T>
struct single_dispatcher_impl<T> : dispatch_holder<T> {
   using dispatch_holder<T>::dispatch_holder;
   using dispatch_holder<T>::operator();
};

template <class T, class... Ts>
struct single_dispatcher_impl<T, Ts...> : dispatch_holder<T>, single_dispatcher_impl<Ts...> {
   using dispatch_holder<T>::operator();
   using details_::single_dispatcher_impl<Ts...>::operator();

protected:
   explicit single_dispatcher_impl(T &&t, Ts &&... ts)
      : dispatch_holder<T>{std::forward<T>(t)},
        single_dispatcher_impl<Ts...>{std::forward<Ts>(ts)...} {
   }
};
}

template <class... Fs>
struct single_dispatcher final : details_::single_dispatcher_impl<std::decay_t<Fs>...> {
   using details_::single_dispatcher_impl<std::decay_t<Fs>...>::operator();

   explicit single_dispatcher(std::decay_t<Fs> &&... fs)
      : details_::single_dispatcher_impl<std::decay_t<Fs>...>{std::move(fs)...} {
   }
};

template <class... Fs>
single_dispatcher<Fs...> make_single_dispatcher(Fs... fs) {
   return single_dispatcher<Fs...>{std::move(fs)...};
}

namespace details_ {
template <class Signature>
struct handler;

template <class R, class... A>
struct handler<R(A...)> {
   virtual ~handler() noexcept {
   }
   virtual R handle(A...) = 0;
};

template <class... Signatures>
struct base_handler : handler<Signatures>... {};

template <class, class, class...>
struct handler_impl;

template <class Impl, class... Signatures>
struct handler_impl<Impl, parameter_pack<Signatures...>> : base_handler<Signatures...> {
   explicit handler_impl(Impl &&i) noexcept(std::is_move_constructible<Impl>::value)
      : impl(std::move(i)) {
   }

protected:
   Impl &get() noexcept {
      return impl;
   }
   Impl const &get() const noexcept {
      return impl;
   }

private:
   Impl impl;
};

template <class Impl, class... Signatures, class R, class... A, class... Tail>
struct handler_impl<Impl, parameter_pack<Signatures...>, R(A...), Tail...>
    : handler_impl<Impl, parameter_pack<Signatures...>, Tail...> {
   using handler_impl<Impl, parameter_pack<Signatures...>, Tail...>::handler_impl;

   virtual R handle(A... a) override final {
      return std::invoke(this->get(), a...);
   }
};

template <bool, template <class, class> class, class, class>
struct type_count {
   constexpr static size_t value = 0;
};

template <template <class, class> class Filter>
struct type_count<true, Filter, parameter_pack<>, parameter_pack<>> {
   constexpr static size_t value = 1;
};

template <template <class, class> class Filter, class A, class... As, class B, class... Bs>
struct type_count<true, Filter, parameter_pack<A, As...>, parameter_pack<B, Bs...>> {
   constexpr static size_t value =
       (Filter<std::decay_t<A>, std::decay_t<B>>::value ? 1 : 0) +
       type_count<true, Filter, parameter_pack<As...>, parameter_pack<Bs...>>::value;
};

template <class, class...>
class handler_picker {
public:
   constexpr static size_t count = 0;
   using handler_type = void;
   using result_type = void;
};

template <class... InputArgs, class R, class... A, class... Tail>
class handler_picker<parameter_pack<InputArgs...>, R(A...), Tail...> {
   using next = handler_picker<parameter_pack<InputArgs...>, Tail...>;
   using handler_base = handler<R(A...)>;
   using handler_func = R (handler_base::*)(A...);

   constexpr static bool is_valid =
       is_callable_v<handler_func(handler<R(A...)> *, InputArgs...), R>;
   constexpr static size_t current_count =
       type_count<is_valid, std::is_same, parameter_pack<InputArgs...>,
                  parameter_pack<A...>>::value;
   constexpr static bool use_mine = current_count > next::count;

public:
   constexpr static size_t count = use_mine ? current_count : next::count;
   using handler_type = std::conditional_t<use_mine, handler_base, typename next::handler_type>;
   using result_type = std::conditional_t<use_mine, R, typename next::result_type>;
};
}

template <class... Signatures>
class multi_dispatcher {
   using handler_type = details_::base_handler<Signatures...>;
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

   std::experimental::pmr::vector<handler_ptr> handlers;

   std::experimental::pmr::memory_resource *resource() const noexcept {
      return handlers.get_allocator().resource();
   }

public:
   explicit multi_dispatcher(std::experimental::pmr::memory_resource *resource) noexcept
       : handlers{resource == nullptr ? std::experimental::pmr::get_default_resource() : resource} {
   }

   explicit multi_dispatcher() noexcept
       : multi_dispatcher(std::experimental::pmr::get_default_resource()) {
   }

   // Currently, the signatures must match exactly.  Otherwise, it'll fail to resolve as
   // dispatchable. We should first check to see if we have an exact match.
   template <class... A>
   void operator()(A... a) {
      using match = details_::handler_picker<parameter_pack<A...>, Signatures...>;
      using base = typename match::handler_type;
      static_assert(!std::is_void<std::remove_pointer_t<base>>::value,
                    "There was no handler found to support the called arguments.");
      std::for_each(handlers.begin(), handlers.end(), [&](handler_ptr &handler) {
         static_cast<base *>(handler.get())->handle(a...);
      });
   }

   template <class F>
   void add_handler(F f) {
      using Impl =
          details_::handler_impl<std::decay_t<F>, parameter_pack<Signatures...>, Signatures...>;

      Impl *const handler = static_cast<Impl *>(resource()->allocate(sizeof(Impl), alignof(Impl)));
      new (handler) Impl(std::forward<F>(f));
      handlers.emplace_back(handler, handler_deleter{sizeof(Impl), alignof(Impl), resource()});
   }

   void shrink_to_fit() {
      handlers.shrink_to_fit();
   }

   typename std::experimental::pmr::vector<handler_ptr>::size_type size() const noexcept {
      return handlers.size();
   }

   bool empty() const noexcept {
      return handlers.empty();
   }
};
}
