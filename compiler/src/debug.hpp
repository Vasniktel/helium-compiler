//
// Created by vasniktel on 27.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_DEBUG_HPP_
#define HELIUM_COMPILER_SRC_DEBUG_HPP_

#ifdef HELIUM_COMPILER_DEBUG

#include <iostream>
#include "parser/parser.hpp"

namespace helium {

std::ostream& operator <<(std::ostream& os, const Token& token);

}

#endif

#endif //HELIUM_COMPILER_SRC_DEBUG_HPP_
