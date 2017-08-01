#pragma once

#include <domains/utils/type_traits.hpp>

namespace domains {
// Core Concepts
template <typename T>
concept bool Same = std::is_same<T, T>::value;
// NOTE: This is probably wrong, but should hopefully fullfil our simple purposes
template <typename T, typename U>
concept bool Same() {
   return Same<T>() && Same<U>() && std::is_same<T, U>::value && std::is_same<U, T>::value;
}

// NOTE: This is probably wrong, but should hopefully fullfil our simple purposes
template <typename T, typename U>
concept bool DerivedFrom = std::is_base_of<T, U>::value;

template <typename T, typename U>
concept bool ConvertibleTo = std::is_convertible<T, U>::value;

template <typename T, typename U>
concept bool CommonReference() {
   return requires(T(&t)(), U(&u)()) {
      typename common_reference_t<T, U>;
      typename common_reference_t<U, T>;
      requires Same<common_reference_t<T, U>, common_reference_t<U, T>>;
      common_reference_t<T, U>(t());
      common_reference_t<T, U>(u());
   };
   }

   template <typename T, typename U>
   concept bool Common() {
      return CommonReference<T, U>() && requires(T(&t)(), U(&u)()) {
         typename common_type_t<T, U>;
         typename common_type_t<U, T>;
         requires Same<common_type_t<T, U>, common_type_t<U, T>>;
         common_type_t<T, U>(t());
         common_type_t<T, U>(u());
         requires CommonReference<std::add_lvalue_reference_t<common_type_t<T, U>>,
                                  common_reference_t<std::add_lvalue_reference_t<T const>,
                                                     std::add_lvalue_reference_t<U const>>>();
      };
   }

   template <typename T>
   concept bool Integral = is_integral_v<T>;

   template <typename T>
   concept bool SignedIntegral = Integral<T>() && is_signed_v<T>;

   template <typename T>
   concept bool UnsignedIntegral = Integral<T>() && is_unsigned_v<T>;

   template <typename T, typename U>
   concept bool Assignable() {
      return CommonReference<T const &, U const &>() && requires(T && t, U && t) {
         { std::forward<T>(t) = std::forward<U>(u) }
         ->Same<T &>;
      };
   }

   namespace details_ {
   template <class T, class U>
   void swap(T &&t, U &&u) noexcept {
      using std::swap;
      swap(std::forward<T>(t), std::forward<U>(u));
   }
   }

   template <typename T>
   concept bool Swappable() {
      return requires(T && a, T && b) {
         details_::swap(std::forward<T>(a), std::forward<T>(b));
      };
   }

   template <typename T, typename U>
   concept bool Swappable() {
      return Swappable<T>() && Swappable<U>() && CommonReference<T const &, U const &>() &&
             requires(T && t, U && u) {
         details_::swap(std::forward<T>(t), std::forward<U>(u));
         details_::swap(std::forward<U>(u), std::forward<T>(t));
      };
   }


// Object Concepts (part 1)
template<typename T>
concept bool Destructible() {
   return requires(T t, T const ct, T *p) {
      { t.~T() } noexcept;
      { &t } -> Same<T *>;
      { &ct } -> Same<T const *>;
      delete p;
      delete[] p;
   };
}

namespace details_ {
template <typename T, typename... Args>
concept bool ConstructibleObject() {
   return Destructible<T>() && requires(Args && ... args) {
      T{std::forward<Args>(args)...};
      new T{std::forward<Args>(args)...};
   };
}

template <typename T, typename... Args>
concept bool BindableReference() {
   return std::is_reference<T>::value && requires(Args && ... args) {
      T(std::forward<Args>(args)...);
   };
}
}

template <typename T, typename... Args>
concept bool Constructible() {
   return details_::ConstructibleObject<T, Args...>() || details_::BindableReference<T, Args...>();
}

template <typename T>
concept bool DefaultConstructible() {
   return Constructible<T>() && requires(size_t const n) {
      new T[n]{};
   };
}

template <typename T>
concept bool MoveConstructible() {
   return Constructible<T, std::remove_cv_t<T> &&>() && ConvertibleTo<std::remove_cv_t<T> &&, T>();
}

template <typename T>
concept bool CopyConstructible() {
   return MoveConstructible<T>() && Constructible<T, std::remove_cv_t<T> const &>() &&
          ConvertibleTo<std::remove_cv_t<T> &, T>() &&
          ConvertibleTo<std::remove_cv_t<T> const &, T>() &&
          ConvertibleTo<std::remove_cv_t<T> const &&, T>();
}

template <typename T>
concept bool Movable() {
   return MoveConstructible<T>() && Assignable<T &, T>() && Swappable<T &>();
}

template <typename T>
concept bool Copyable() {
   return CopyConstructible<T>() && Movable<T>() && Assignable<T &, T const &>();
}

template <typename T>
concept bool SemiRegular() {
   return Copyable<T>() && DefaultConstructible<T>();
}

// Comparison Concepts
template<typename B>
concept bool Boolean() {
   return MoveConstructible<B>() &&
      requires(B const b1, B const b2, bool const a) {
         bool(b1);
         { b1 } -> bool;
         bool(!b1);
         { !b1 } -> bool;
         { b1 && b2 } -> Same<bool>;
         { b1 && a } -> Same<bool>;
         { a && b1 } -> Same<bool>;
         { b1 || b2 } -> Same<bool>;
         { b1 || a } -> Same<bool>;
         { a || b1 } -> Same<bool>;
         { b1 == b2 } -> bool;
         { b1 != b2 } -> bool;
         { b1 == a } -> bool;
         { a == b1 } -> bool;
         { b1 != a } -> bool;
         { a != b1 } -> bool;
      };
}

template <typename T, typename U>
concept bool WeaklyEqualityComparable() {
   return requires(T const &t, U const &u) {
      { t == u }
      ->Boolean;
      { u == t }
      ->Boolean;
      { t != u }
      ->Boolean;
      { u != t }
      ->Boolean;
   };
}

template <typename T>
concept bool EqualityComparable() {
   return WeaklyEqualityComparable<T, T>();
}

template <typename T, typename U>
concept bool EqualityComparable() {
   return CommonReference<T const &, U const &>() && EqualityComparable<T>() &&
          EqualityComparable<U>() &&
          EqualityComparable<std::remove_cv_t<
              std::remove_reference_t<common_reference_t<T const &, U const &>>>>() &&
          WeaklyEqualityComparable<T, U>();
}

template <typename T>
concept bool StrictlyTotallyOrdered() {
   return EqualityComparable<T>() && requires(T const a, T const b) {
      { a < b }
      ->Boolean;
      { a > b }
      ->Boolean;
      { a <= b }
      ->Boolean;
      { a >= b }
      ->Boolean;
   };
}

template <typename T, typename U>
concept bool StrictlyTotallyOrdered() {
   return CommonReference<T const &, U const &>() && StrictlyTotallyOrdered<T>() &&
          StrictlyTotallyOrdered<U>() &&
          StrictlyTotallyOrdered<std::remove_cv_t<
              std::remove_reference_t<common_reference_t<T const &, U const &>>>>() &&
          EqualityComparable<T, U>() && requires(T const t, U const u) {
      { t < u }
      ->Boolean;
      { t > u }
      ->Boolean;
      { t <= u }
      ->Boolean;
      { t >= u }
      ->Boolean;
      { u < t }
      ->Boolean;
      { u > t }
      ->Boolean;
      { u <= t }
      ->Boolean;
      { u >= t }
      ->Boolean;
   };
}

// Object Concepts (part 2)
template <typename T>
concept bool Regular() {
   return SemiRegular<T>() && EqualityComparable<T>();
}

// Iterator Concepts
template <typename I>
concept bool Readable() {
   return Movable<I>() && DefaultConstructible<I>() && requires(I const &i) {
      typename value_type_t<I>;
      typename reference_t<I>;
      typename rvalue_reference_t<I>;
      { *i }
      ->Same<reference_t<I>>;
      { iter_move(i) }
      ->Same<rvalue_reference_t<I>>;
   }
   &&CommonReference<reference_t<I>, value_type_t<I> &>() &&
       CommonReference<reference_t<I>, rvalue_reference_t<I>>() &&
       CommonReference<rvalue_reference_t<I>, value_type<I> const &>() &&
       Same<common_reference_t<reference_t<I>, value_type_t<I>>, value_type_t<I>>() &&
       Same<common_reference_t<rvalue_reference_t<I>, value_type_t<I>>, value_type_t<I>>();
}

template <typename Out, typename In>
concept bool Writable() {
   return SemiRegular<Out>() && requires(Out out, In && in) {
      *out = std::forward<In>(in);
   };
}

template <typename I>
concept bool WeaklyIncrementable() {
   return SemiRegular<I>() && requires(I i) {
      typename difference_type_t<I>;
      requires SignedIntegral<difference_type_t<I>>();
      { ++i }
      ->Same<I &>;
      i++; // Why is this here?
   };
}

template <typename I>
concept bool Incrementable() {
   return Regular<I>() && WeaklyIncrementable<I>() && requires(I i) {
      { i++ }
      ->Same<I>;
   };
}

template <typename I>
concept bool Iterator() {
   return WeaklyIncrementable<I>() && requires(I i) {
      {*i};
   };
}

template <typename S, typename I>
concept bool Sentinel() {
   return SemiRegular<S>() && Iterator<I>() && WeaklyEqualityComparable<S, I>();
}

template <typename S, typename I>
concept bool SizedSentinel() {
   return Sentinel<S, I>() && requires(I const &i, S const &s) {
      { s - i }
      ->Same<difference_type_t<I>>;
      { i - s }
      ->Same<difference_type_t<I>>;
   };
}

template <typename I>
concept bool InputIterator() {
   return Iterator<I>() && Readable<I>() && requires(I i, I const ci) {
      typename iterator_category_t<I>;
      requires DerivedFrom<iterator_category_t<I>, std::input_iterator_tag>();
      { i++ }
      ->Readable;
      requires Same<value_type_t<I>, value_type_t<decltype(i++)>>;
      { *ci }
      ->value_type_t<I> const &;
   };
}

template <typename I, typename T>
concept bool OutputIterator() {
   return Iterator<I>() && Writable<I, T>();
}

template <typename I>
concept bool ForwardIterator() {
   return InputIterator<I>() && DerivedFrom<iterator_category_t<I>, std::forward_iterator_tag>() &&
          Incrementable<I>() && Sentinel<I, I>();
}

template <typename I>
concept bool BidirectionalIterator() {
   return ForwardIterator<I>() &&
          DerivedFrom<iterator_category_t<I>, std::bidirectional_iterator_tag>() && requires(I i) {
      { --i }
      ->Same<I &>;
      { i-- }
      ->Same<I>;
   };
}

template <typename I>
concept bool RandomAccessIterator() {
   return BidirectionalIterator<I>() &&
          DerivedFrom<iterator_category_t<I>, std::random_access_iterator_tag>() &&
          StrictlyTotallyOrdered<I>() && SizedSentinel<I, I>() &&
          requires(I i, I const j, difference_type_t<I> const n) {
      { i += n } ->Same<I &>;
      { j + n } ->Same<I>;
      { n + j } ->Same<I>;
      { i -= n } ->Same<I &>;
      { j - n } ->Same<I>;
      { j[n] } ->Same<reference_t<I>>;
   };
}

namespace details_ {
template <class T>
void begin(T &&t) noexcept {
   using std::begin;
   begin(std::forward<T>(t));
}

template <class T>
void end(T && t) noexcept {
   using std::end;
   end(std::forward<T>(t));
}
}

template <typename T>
concept bool Range() {
   return requires(T && t) {
      details_::end(t);
   };
}

template <typename T>
concept bool SizedRange() {
   return Range<T>() && requires(std::remove_reference_t<T> const &t) {
      { size(t) }
      ->ConvertibleTo<difference_type_t<iterator_t<T>>>;
   };
}

namespace details_ {
template <typename T>
concept bool ContainerLike_() {
   return Range<T>() && Range<T const>() &&
          !Same<reference_t<iterator_t<T>>, reference_t<iterator_t<T const>>>();
}

template <typename T>
constexpr bool view_predicate_ = true;

template <typename T>
requires requires {
   typename enable_view<T>::type;
}
constexpr bool view_predicate_ = enable_view<T>::type::value;

template <typename T>
requires !(DerivedFrom<T view_base>() || requires{typename enable_view<T>::type})
constexpr bool view_predicate_ = false;
}

template <typename T>
concept bool View() {
   return Range<T>() && SemiRegular<T>() && details_::view_predicate_<T>;
}

template <typename T>
concept bool BoundedRange() {
   return Range<T>() && Same<iterator_t<T>, sentinel_t<T>>();
}

template <typename T>
concept bool InputRange() {
   return Range<T>() && InputIterator<iterator_t<T>>();
}

template <typename R, typename T>
concept bool OutputRange() {
   return Range<T>() && OutputIterator<iterator_t<R>, T>();
}

template <typename T>
concept bool ForwardRange() {
   return Range<T>() && ForwardIterator<iterator_t<T>>();
}

template <typename T>
concept bool BidirectionalRange() {
   return Range<T>() && BidirectionalIterator<iterator_t<T>>();
}

template <typename T>
concept bool RandomAccessRange() {
   return Range<T>() && RandomAccessIterator<iterator_t<T>>();
}

// Callable Concepts
template <typename F, typename... Args>
concept bool Invocable() {
   return CopyConstructible<F>() && requires(F f, Args && ... args) {
      std::invoke(f, std::forward<Args>(args)...);
   };
}

template <typename F, typename... Args>
concept bool RegularInvocable = Invocable<F, Args...>();

template <typename F, typename... Args>
concept bool Predicate = Invocable<F, Args...>() && Boolean<std::result_of_t<F &(Args...)>>();

template <typename F, typename T>
concept bool Relation = Predicate<F, T, T>();

template <typename F, typename T, typename U>
concept bool Relation() {
   return Relation<F, T>() && Relation<F, U>() && CommonReference<T const &, U const &>() &&
          Relation<F, common_reference_t<T const &, U const &>>() && Predicate<F, T, U>() &&
          Predicate<F, U, T>();
}

template <typename F, typename T>
concept bool StrictWeakOrder = Relation<F, T>();

template <typename F, typename T, typename U>
concept bool StrictWeakOrder = Relation<F, T, U>();

template <typename G>
concept bool UniformRandomNumberGenerator() {
   return requires(G g) {
      { g() }
      ->ranges::UnsignedIntegral;
      { G::min() }
      ->ranges::Same<std::result_of_t<G &()>>;
      { G::max() }
      ->ranges::Same<std::result_of_t<G &()>>;
   };
}
}
