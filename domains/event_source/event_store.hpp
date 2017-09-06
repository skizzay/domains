#pragma once

#include <experimental/unordered_map>
#include <experimental/vector>

#include <system_error>

#include "domains/event_source/event_stream.hpp"

namespace domains {

template<class T>
concept bool EventStore() {
   return requires(T const &tc, T &t) {
      typename T::id_type;
      { tc.num_events() } -> UnsignedIntegral;
      { tc.get_events() } -> EventStream;
   };
}

namespace details_ {
template <class EventType>
std::experimental::pmr::vector<EventType> const &empty_record() noexcept {
   static std::experimental::pmr::vector<EventType> const singleton;
   return singleton;
}

template <class Iterator>
class memory_event_stream final {
   Iterator b, e;

public:
   explicit memory_event_stream(Iterator b_, Iterator e_) noexcept : b{b_}, e{e_} {
   }

   Iterator begin() const noexcept {
      return b;
   }

   Iterator end() const noexcept {
      return e;
   }

   bool empty() const noexcept {
      return begin() == end();
   }

   std::make_unsigned_t<typename std::iterator_traits<Iterator>::difference_type> size() const
       noexcept {
      return std::distance(begin(), end());
   }
};
}

template <class IdType, class EventType>
class memory_store final {
   using record_type = std::experimental::pmr::vector<EventType>;
   using table_type = std::experimental::pmr::unordered_map<IdType, record_type>;

   table_type table;

   inline std::experimental::pmr::memory_resource *resource() const noexcept {
      return table.get_allocator().resource();
   }

public:
   using id_type = IdType;
   using event_type = EventType;
   using size_type = typename record_type::size_type;
   using event_stream = details_::memory_event_stream<typename record_type::const_iterator>;

   memory_store() noexcept = default;

   explicit memory_store(std::experimental::pmr::memory_resource *r) noexcept
       : table{r == nullptr ? std::experimental::pmr::get_default_resource() : r} {
   }

   size_type num_events(IdType const id) const noexcept {
      auto events = table.find(id);
      return events == table.end() ? 0 : events->second.size();
   }

   event_stream get_events(IdType const id, size_type const start_event_num) const noexcept {
      auto events = table.find(id);
      if (events == table.end()) {
         return event_stream({}, {});
      }
      return event_stream(
          std::next(std::cbegin(events->second), std::min(events->second.size(), start_event_num)),
          std::cend(events->second));
   }

   std::error_code save(IdType const id, EventType event) noexcept {
      try {
         auto events = table.find(id);
         if (events == table.end()) {
            table.emplace(id, record_type(1, std::move(event), resource()));
         } else {
            events->second.emplace_back(std::move(event));
         }
         return {};
      } catch (std::bad_alloc const &) {
         return make_error_code(std::errc::not_enough_memory);
      }
   }
};

struct null_event_store final {
   template <class IdType>
   auto num_events(IdType const) const noexcept {
      return 0U;
   }

   template <class IdType>
   std::array<char, 0> get_events(IdType const, std::uint64_t const) const noexcept {
      return {};
   }

   template <class IdType, class EventType>
   std::error_code save(IdType const, EventType const &) const noexcept {
      return {};
   }
};
}
