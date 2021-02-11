#include "domains/messaging/translator.hpp"

#include "domains/messaging/dispatcher.hpp"
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <algorithm>
#include <experimental/string_view>
#include <iostream>

using namespace domains;

struct A {
   virtual ~A() noexcept = default;
};

struct B final : A {};

struct C : A {};

struct D final : C {};

struct fixture final {};

TEST_CASE("static cast translator", "[messaging][translator]") {
   static_casting_translator target;
   size_t a_count = 0;
   size_t b_count = 0;
   size_t c_count = 0;
   size_t d_count = 0;

   auto domain_dispatcher = single_dispatcher(
       [&](A const &) -> std::error_code {
          ++a_count;
          return {};
       },
       [&](B const &) -> std::error_code {
          ++b_count;
          return {};
       },
       [&](C const &) -> std::error_code {
          ++c_count;
          return {};
       },
       [&](D const &) -> std::error_code {
          ++d_count;
          return {};
       });

   auto a_to_b_router = [](A const &a, auto dispatcher) noexcept->std::error_code {
      return dispatcher.template decode_and_dispatch<B>(a);
   };

   auto a_to_d_router = [](A const &a, auto dispatcher) noexcept->std::error_code {
      return dispatcher.template decode_and_dispatch<D>(a);
   };

   auto c_to_d_router = [](C const &c, auto dispatcher) noexcept->std::error_code {
      return dispatcher.template decode_and_dispatch<D>(c);
   };

   SECTION("will dispatch to child class") {
      B b;
      a_to_b_router(b, target.dispatch_to(domain_dispatcher));
      REQUIRE(b_count == 1);
   }

   SECTION("will dispatch to grandchild class") {
      D d;
      a_to_d_router(d, target.dispatch_to(domain_dispatcher));
      REQUIRE(d_count == 1);
   }

   SECTION("will dispatch child to grandchild class") {
      D d;
      c_to_d_router(d, target.dispatch_to(domain_dispatcher));
      REQUIRE(d_count == 1);
   }
}

TEST_CASE("dynamic cast translator", "[messaging][translator]") {
   dynamic_casting_translator target;
   size_t a_count = 0;
   size_t b_count = 0;
   size_t c_count = 0;
   size_t d_count = 0;

   auto domain_dispatcher = single_dispatcher(
       [&](A const &) -> std::error_code {
          ++a_count;
          return {};
       },
       [&](B const &) -> std::error_code {
          ++b_count;
          return {};
       },
       [&](C const &) -> std::error_code {
          ++c_count;
          return {};
       },
       [&](D const &) -> std::error_code {
          ++d_count;
          return {};
       });

   auto a_to_b_router = [](A const &a, auto dispatcher) noexcept->std::error_code {
      return dispatcher.template decode_and_dispatch<B>(a);
   };

   auto a_to_d_router = [](A const &a, auto dispatcher) noexcept->std::error_code {
      return dispatcher.template decode_and_dispatch<D>(a);
   };

   auto c_to_d_router = [](C const &c, auto dispatcher) noexcept->std::error_code {
      return dispatcher.template decode_and_dispatch<D>(c);
   };

   SECTION("will dispatch to child class") {
      B b;
      a_to_b_router(b, target.dispatch_to(domain_dispatcher));
      REQUIRE(b_count == 1);
   }

   SECTION("will dispatch to grandchild class") {
      D d;
      a_to_d_router(d, target.dispatch_to(domain_dispatcher));
      REQUIRE(d_count == 1);
   }

   SECTION("will dispatch child to grandchild class") {
      D d;
      c_to_d_router(d, target.dispatch_to(domain_dispatcher));
      REQUIRE(d_count == 1);
   }
}
