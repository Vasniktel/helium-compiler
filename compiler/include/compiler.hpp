//
// Created by vasniktel on 26.08.2019.
//

#ifndef HELIUM_COMPILER_INCLUDE_COMPILER_HPP_
#define HELIUM_COMPILER_INCLUDE_COMPILER_HPP_

#ifndef HELIUM_COMPILER_DEBUG
#define NDEBUG
#endif

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include <ostream>
#include <string>
#include <vector>
#include <queue>

namespace helium {

class Compiler final {
 private:

  Compiler() = default;
  static absl::optional<std::string> Compile(
      const std::string& name, const std::string& source, std::vector<uint8_t>& out);

 public:
  static absl::optional<std::string> FromFile(const std::string& name, std::vector<uint8_t>& out);
  static absl::optional<std::string> FromSource(const std::string& source, std::vector<uint8_t>& out) {
    return Compile("<source string>", source, out);
  }
};

} // namespace helium

#endif //HELIUM_COMPILER_INCLUDE_COMPILER_HPP_
