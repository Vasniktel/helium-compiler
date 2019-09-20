//
// Created by vasniktel on 20.09.2019.
//

#include "gtest/gtest.h"
#include "interner.hpp"
#include "absl/strings/string_view.h"

namespace helium {

TEST(Interner, Intern) {
  using ::absl::string_view;

  Interner interner;
  ASSERT_EQ(interner.Intern("hello"), 0);
  ASSERT_EQ(interner.Intern("world"), 1);

  ASSERT_TRUE(interner.LookUp(0));
  ASSERT_EQ(*interner.LookUp(0), string_view{"hello"});

  ASSERT_TRUE(interner.LookUp(1));
  ASSERT_EQ(*interner.LookUp(1), string_view{"world"});

  ASSERT_FALSE(interner.LookUp(2));
}

}
