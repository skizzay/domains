#pragma once

#include <coroutine>
#include <exception>
#include <iterator>
#include <type_traits>

namespace skizzay::domains::event_source {

inline namespace generator_v1 {

template<typename> class generator;

namespace generator_details_ {

template<typename T>
class generator_promise {
public:
   using value_type = std::remove_reference_t<T>;
   using reference = std::conditional_t<std::is_reference_v<T>, T, std::add_lvalue_reference_t<T>>;
   using pointer = std::add_pointer_t<value_type>;

   generator<T> get_return_object() noexcept;

   void return_void() noexcept {
   }

   void unhandled_exception() noexcept {
      e_ = std::current_exception();
   }

   std::suspend_always initial_suspend() const noexcept {
      return {};
   }

   std::suspend_always final_suspend() const noexcept {
      return {};
   }

   template<typename U=T, std::enable_if_t<!std::is_rvalue_reference_v<T>, int> = 0>
   std::suspend_always yield_value(std::remove_reference_t<T> &value) noexcept {
      value_ = std::addressof(value);
      return {};
   }

   std::suspend_always yield_value(std::remove_reference_t<T> &&value) noexcept {
      value_ = std::addressof(value);
      return {};
   }
   
   // Don't allow any use of 'co_await' inside the generator coroutine.
   template<typename U>
   std::suspend_never await_transform(U&& value) = delete;

   reference value() const noexcept {
      return static_cast<reference>(*value_);
   }

   void rethrow_if_exception() {
      if (nullptr != e_) {
         std::rethrow_exception(e_);
      }
   }

private:
   std::exception_ptr e_ = nullptr;
   pointer value_ = nullptr;
};

template<typename T>
void retrieve_next_or_throw(std::coroutine_handle<generator_promise<T>> const &coroutine) {
   coroutine.resume();
   if (coroutine.done()) {
      coroutine.promise().rethrow_if_exception();
   }
}
}

template<typename T>
class [[nodiscard]] generator {
public:
   using promise_type = generator_details_::generator_promise<T>;

   class iterator {
   public:
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = typename promise_type::value_type;
      using reference = typename promise_type::reference;
      using pointer = typename promise_type::pointer;

      iterator() noexcept = default;

      bool operator==(iterator const &r) const noexcept {
         return this->coroutine_ == r.coroutine_;
      }

      bool operator!=(iterator const &r) const noexcept {
         return this->coroutine_ != r.coroutine_;
      }

      bool operator==(std::default_sentinel_t) const noexcept {
         return !coroutine_ || coroutine_.done();
      }

      bool operator!=(std::default_sentinel_t) const noexcept {
         return coroutine_ && !coroutine_.done();
      }

      iterator & operator++() {
         generator_details_::retrieve_next_or_throw(coroutine_);
         return *this;
      }

      void operator++(int) {
         (void)operator++();
      }

      reference operator*() const noexcept {
         return value();
      }

      pointer operator->() const noexcept {
         return std::addressof(value());;
      }

   private:
      friend class generator<T>;

      explicit iterator(std::coroutine_handle<promise_type> coroutine) noexcept :
         coroutine_{std::move(coroutine)}
      {
      }

      std::coroutine_handle<promise_type> coroutine_;

      reference value() const noexcept {
         return coroutine_.promise().value();
      }
   };

   generator() noexcept :
      coroutine_{nullptr}
   {
   }

   generator(generator &&other) noexcept :
      coroutine_{std::move(other.coroutine_)}
   {
      other.coroutine_ = nullptr;
   }

   generator(generator const &) = delete;

   ~generator() noexcept {
      destroy_coroutine();
   }

   generator & operator =(generator &&other) noexcept {
      if (this != &other) {
         destroy_coroutine();
         coroutine_ = std::move(other.coroutine_);
         other.coroutine_ = nullptr;
      }
      return *this;
   }

   void operator =(generator const &) = delete;

   iterator begin() const noexcept {
      if (coroutine_) {
         generator_details_::retrieve_next_or_throw(coroutine_);
      }
      return iterator{coroutine_};
   }

   auto end() const noexcept {
      return std::default_sentinel;
   }

   void swap(generator<T> &&other) noexcept {
      std::swap(coroutine_, other.coroutine_);
   }

private:
   friend class generator_details_::generator_promise<T>;

   std::coroutine_handle<promise_type> coroutine_;

   explicit generator(std::coroutine_handle<promise_type> handle) noexcept :
      coroutine_{std::move(handle)}
   {
   }

   void destroy_coroutine() noexcept {
      if (coroutine_) {
         coroutine_.destroy();
      }
   }
};

template<typename T>
void swap(generator<T> &&l, generator<T> &&r) noexcept {
   l.swap(r);
}

namespace generator_details_ {
template<typename T>
generator<T> generator_promise<T>::get_return_object() noexcept {
   return generator<T>{std::coroutine_handle<generator_promise<T>>::from_promise(*this)};
}
}
}

}