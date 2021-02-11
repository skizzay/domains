#pragma once

#include <domains/utils/null_mutex.hpp>
#include <domains/utils/parameter_pack.hpp>

#include <memory>
#include <mutex>
#include <type_traits>

namespace domains {
namespace details_ {
template <class Mutex>
class destroy_and_unlock {
   std::unique_lock<Mutex> lock;

public:
   destroy_and_unlock(std::unique_lock<Mutex> &&l) noexcept : lock(std::move(l)) {
   }

   template <class T>
   void operator()(T *const t) noexcept {
      t->~T();
      lock.unlock();
   }
};
}

template <class Mutex, class T>
class basic_single_type_provider {
   using deleter = details_::destroy_and_unlock<Mutex>;
   struct : Mutex {
      std::aligned_storage_t<sizeof(std::decay_t<T>), alignof(std::decay_t<T>)> storage;
   } holder;

public:
   template <class U, class... A>
   std::enable_if_t<std::is_same<std::decay_t<T>, std::decay_t<U>>::value,
                    std::unique_ptr<std::decay_t<U>, deleter>>
   provide(A &&... a) {
      std::unique_lock<Mutex> lock(holder);
      return {new (&holder.storage) std::decay_t<T>(std::forward<A>(a)...), {std::move(lock)}};
   }

protected:
   static basic_single_type_provider<Mutex, T> &
   select(basic_single_type_provider<Mutex, T> &self) noexcept {
      return self;
   }
};

template <class Mutex, class... Ts>
class basic_multi_type_provider : basic_single_type_provider<Mutex, std::decay_t<Ts>>... {
   using deleter = details_::destroy_and_unlock<Mutex>;
   using provided_types = parameter_pack<std::decay_t<Ts>...>;
   static_assert(is_unique_v<provided_types>, "Types must not be listed more than once.");

public:
   template <class T, class... A>
   std::enable_if_t<in_v<std::decay_t<T>, provided_types>, std::unique_ptr<T, deleter>>
   provide(A &&... a) {
      static_assert(
          in_v<std::decay_t<T>, provided_types>,
          "Requested type T is not a type that can be provided (see listed provided types).");
      return basic_single_type_provider<Mutex, std::decay_t<T>>::select(*this).template provide<T>(
          std::forward<A>(a)...);
   }
};

template <class Mutex, class... Ts>
class basic_compact_multi_type_provider {
   using deleter = details_::destroy_and_unlock<Mutex>;
   using provided_types = parameter_pack<std::decay_t<Ts>...>;
   static_assert(is_unique_v<provided_types>, "Types must not be listed more than once.");

   struct : Mutex {
      std::aligned_union_t<0, std::decay_t<Ts>...> storage;
   } holder;

public:
   template <class T, class... A>
   std::enable_if_t<in_v<std::decay_t<T>, provided_types>, std::unique_ptr<T, deleter>>
   provide(A &&... a) {
      static_assert(
          in_v<std::decay_t<T>, provided_types>,
          "Requested type T is not a type that can be provided (see listed provided types).");
      std::unique_lock<Mutex> lock(holder);
      return {new (&holder.storage) std::decay_t<T>(std::forward<A>(a)...), std::move(lock)};
   }
};

template <class T>
using single_type_provider = basic_single_type_provider<null_mutex, T>;

template <class... Ts>
using multi_type_provider = basic_multi_type_provider<null_mutex, Ts...>;

template <class... Ts>
using compact_multi_type_provider = basic_compact_multi_type_provider<null_mutex, Ts...>;

template <class T>
using shared_single_type_provider = basic_single_type_provider<std::mutex, T>;

template <class... Ts>
using shared_multi_type_provider = basic_multi_type_provider<std::mutex, Ts...>;

template <class... Ts>
using shared_compact_multi_type_provider = basic_compact_multi_type_provider<std::mutex, Ts...>;

class null_type_provider_t {
public:
   template <class T, class... A>
   std::unique_ptr<std::decay_t<T>> provide(A &&...) noexcept {
      return std::unique_ptr<std::decay_t<T>>(nullptr);
   }
};
}
