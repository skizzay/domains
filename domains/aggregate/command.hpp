#pragma once

#include <domains/aggregate/aggregate_repository.hpp>

namespace domains {

template <class T, T>
class command;

template <class A, class R, class T, R (A::*handler)(T)>
class command<R (A::*F)(T), F handler> {
   aggregate_repository<A> *repository;

public:
   explicit command(aggregate_repository<A> &repo) : repository(&repo) {
   }

   R operator()(T t) {
      auto entity = aggregate_repository->get(extract_id(t));
      return ((*entity).*handler)(t);
   }
};
}
