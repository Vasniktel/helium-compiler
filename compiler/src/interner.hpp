//
// Created by vasniktel on 09.09.2019.
//

#ifndef HELIUM_COMPILER_SRC_INTERNER_HPP_
#define HELIUM_COMPILER_SRC_INTERNER_HPP_

#include "absl/container/flat_hash_map.h"
#include <absl/strings/string_view.h>

namespace helium {

class Interner {
  ::absl::flat_hash_map<::absl::string_view, size_t> map_;
  ::std::vector<::absl::string_view> vec_;

 public:
  using Data = size_t;

  Data Intern(::absl::string_view s) {
    auto it = map_.find(s);
    if (it != map_.end()) return it->second;
    map_[s] = vec_.size();
    vec_.push_back(s);
    return vec_.size() - 1;
  }

  ::absl::optional<::absl::string_view> LookUp(Data data) const {
    return data < vec_.size() ? ::absl::make_optional(vec_[data]) : ::absl::nullopt;
  }
};

}

#endif //HELIUM_COMPILER_SRC_INTERNER_HPP_
