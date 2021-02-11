#pragma once

#include <domains/aggregate/entity.hpp>
#include <domains/event_source/event_store.hpp>
#include <domains/messaging/decoder.hpp>

#include <array>
#include <cstdint>
#include <system_error>

namespace domains {
auto get_event_dispatcher(Entity &e) noexcept -> decltype(e.event_dispatcher()) {
   return e.event_dispatcher();
}

Entity &get_event_dispatcher(Entity &e) noexcept {
   return e;
}

template <class Decoder, class Encoder, class EventStore>
class event_source final {
   Decoder decode;
   Encoder encode;
   EventStore store;

public:
   event_source(Decoder d={}, Encoder e={}, EventStore s={})
      : decode{std::move(d)},
        encode{std::move(e)},
        store{std::move(s)} {
   }

   template <class IdType>
   auto num_events(IdType const id) const noexcept {
      return store.num_events(id);
   }

   void build(Entity &entity) noexcept {
#if 1
      auto event_dispatcher = get_event_dispatcher(entity);
      for (auto const &event : store.get_events(entity.id(), entity.version())) {
         decode(event_dispatcher, event);
      }
#else
      build(entity, entity.version());
   }

   void build(Entity &entity, decltype(entity.version()) const start_event_num) noexcept {
      auto event_dispatcher = get_event_dispatcher(entity);
      for (auto const &event : store.get_events(entity.id(), start_event_num)) {
         decode(event_dispatcher, event);
      }
#endif
   }

   template <class IdType, class EventType>
   std::error_code put(IdType const id, EventType const &event) noexcept {
      return store.save(id, encode(event));
   }
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

template <class T>
using null_event_source_t = event_source<null_decoder_t, null_encoder_t<T>, null_event_store>;
}
