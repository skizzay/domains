#pragma once

#include <array>
#include <typeinfo>

#include <domains/utils/byte.hpp>
#include <domains/utils/type_traits.hpp>

namespace domains {
namespace details_ {

template <class T>
concept bool is_svo_eligible = (sizeof(T) <= sizeof(void *)) && (alignof(T) <= alignof(void *));
}

class any {
   struct interface {
      virtual ~interface() noexcept {
      }
      virtual interface *clone(void *hint = nullptr) const = 0;
      virtual std::type_info const &type() const noexcept = 0;
   };

   template <class T>
   struct implementation final : interface {
      T value;
      explicit implementation(T &&t) noexcept(std::is_nothrow_move_constructible<T>::value)
         : value{std::move(t)} {
      }
      explicit implementation(T const &t) noexcept(std::is_nothrow_copy_constructible<T>::value)
         : value{t} {
      }
      virtual implementation<T> *clone(void *hint) const override {
         if (hint != nullptr) {
            return new (hint) implementation<T>(value);
         }
         return new implementation<T>(value);
      }
      virtual std::type_info const &type() const noexcept override {
         return typeid(T);
      }
   };

   union {
      interface *benny;
      std::array<byte, sizeof(void *) * 2> local;
   };

   bool is_local() const noexcept {
      return !empty() && ((reinterpret_cast<uintptr_t>(benny) & 0x1) == 0);
   }

   interface const *get() const noexcept {
      if (empty()) {
         return nullptr;
      }
      if (is_local()) {
         return reinterpret_cast<interface const *>(&local);
      }
      constexpr uintptr_t MASK = ~0 ^ 0x1;
      return reinterpret_cast<interface const *>(reinterpret_cast<uintptr_t>(benny) & MASK);
   }

   void destruct() noexcept {
      if (is_local()) {
         get()->~interface();
      } else {
         delete get();
      }
   }

   void set_benny(interface *const iface) noexcept {
      benny = reinterpret_cast<interface *>(reinterpret_cast<uintptr_t>(iface) | 0x1);
   }

public:
   any() noexcept {
      benny = nullptr;
   }

   any(any const &a) {
      if (a.empty()) {
         benny = nullptr;
      } else if (a.is_local()) {
         a.get()->clone(&local[0]);
      } else {
         set_benny(a.get()->clone());
      }
   }

   template <class T, typename std::enable_if<!is_same_v<std::decay_t<T>, any>, bool>::type = true>
   any(T &&t) requires !details_::is_svo_eligible<T> {
      set_benny(new implementation<std::decay_t<T>>(std::forward<T>(t)));
   }

   template <class T, typename std::enable_if<!is_same_v<std::decay_t<T>, any>, bool>::type = true>
   any(T &&t) noexcept(
       is_nothrow_constructible_v<implementation<std::decay_t<T>>, decltype(std::forward<T>(t))>)
       requires details_::is_svo_eligible<T> {
      new (&local[0]) implementation<std::decay_t<T>>(std::forward<T>(t));
   }

   ~any() noexcept {
      destruct();
   }

   bool empty() const noexcept {
      return benny == nullptr;
   }

   std::type_info const &type() const noexcept {
      return empty() ? typeid(std::nullptr_t) : get()->type();
   }

   void clear() noexcept {
      destruct();
      benny = nullptr;
   }

   friend void swap(any &l, any &r) noexcept {
      swap(l.local, r.local);
   }

   template <class T>
   friend std::decay_t<T> const *any_cast(any const *a) noexcept {
      return a != nullptr && a->type() == typeid(std::decay_t<T>)
                 ? &static_cast<any::implementation<std::decay_t<T>> const *>(a->get())->value
                 : nullptr;
   }
};

template <class T>
std::decay_t<T> *any_cast(any *a) noexcept {
   return const_cast<std::decay_t<T> *>(
       any_cast<std::decay_t<T> const>(const_cast<any const *>(a)));
}

struct bad_any_cast : std::bad_cast {
   virtual char const *what() const noexcept {
      return "Cannot cast to type from any.";
   }
};

template <class T>
std::decay_t<T> &any_cast(any &a) {
   auto *ptr = any_cast<T>(&a);
   if (ptr == nullptr) {
      throw bad_any_cast();
   }
   return *ptr;
}

template <class T>
std::decay_t<T> const &any_cast(any const &a) {
   auto const *const ptr = any_cast<T>(&a);
   if (ptr == nullptr) {
      throw bad_any_cast();
   }
   return *ptr;
}
}
