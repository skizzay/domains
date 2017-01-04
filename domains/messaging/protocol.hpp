#pragma once

namespace domains {

template <class ProtocolDefinition>
class protocol {
   ProtocolDefinition &impl;

public:
   protocol(ProtocolDefinition &impl_) : impl(impl_) {
   }

   void on_data(void const *data, std::size_t num_bytes) {
      impl.get_domain_handler().dispatch(data, num_bytes, impl.get_message_type(data, num_bytes));
   }
};
}
