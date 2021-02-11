#pragma once

namespace domains {
template <typename T>
concept bool CommitRange = requires(T t) {
   typename T::commit_type;
   { *t.begin() }
   ->typename T::commit_type;
   {t.end()};
};
}
