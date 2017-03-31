#pragma once

#include <domains/aggregate/event_store.hpp>
#include <domains/messaging/decoder.hpp>

#include <array>
#include <cstdint>
#include <system_error>

namespace domains {
template <class Entity>
Entity &get_event_dispatcher(Entity &e) noexcept {
   return e;
}

template <class Entity>
auto get_version(Entity const &e) noexcept -> decltype(e.version()) {
   return e.version();
}

template <class Entity>
unsigned get_version(Entity const &) noexcept {
   return 0U;
}

template <class Decoder, class Encoder, class EventStore>
class event_source final {
public:
   explicit event_source(Decoder &&d = {}, Encoder &&e = {}, EventStore &&s = {})
      : decode{std::move(d)}, encode{std::move(e)}, store{std::move(s)} {
   }

   template <class IdType>
   auto num_events(IdType const id) const noexcept {
      return store.num_events(id);
   }

   template <class Entity>
   void build(Entity &entity, std::uint64_t const start_event_num) noexcept {
      auto event_dispatcher = get_event_dispatcher(entity);
      for (auto const &event : store.get_events(entity.id(), start_event_num)) {
         decode(event_dispatcher, event);
      }
   }

   template <class Entity>
   void build(Entity &entity) noexcept {
      build(entity, get_version(entity));
   }

   template <class IdType, class EventType>
   std::error_code put(IdType const id, EventType const &event) noexcept {
      return store.save(id, encode(event));
   }

private:
   Decoder decode;
   Encoder encode;
   EventStore store;
};

template <class T>
struct null_encoder final {
   template <class U>
   T operator()(U const &) const noexcept(std::is_nothrow_constructible<T>::value) {
      return T();
   }
};

template <class T>
using null_encoder_t = null_encoder<T>;

template <class T, class... U>
using null_event_source_t = event_source<null_decoder_t<U...>, null_encoder_t<T>, null_event_store>;
}
