#pragma once

#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "repl/result.h"

#define CONSUME_OR_RETURN(want)                                                \
  do {                                                                         \
    if (la->type != want) {                                                    \
      std::stringstream out;                                                   \
      out << "unexpected token: got " << la->ToDescString() << ", want "       \
          << Token::TypeToDescString(want);                                    \
      std::vector<std::string> errors;                                         \
      errors.push_back(out.str());                                             \
      return errors;                                                           \
    }                                                                          \
    ++la;                                                                      \
  } while (false)

#define PARSE_OR_RETURN(expr, f)                                               \
  do {                                                                         \
    auto _res = f;                                                             \
    if (!ResultOK(_res)) {                                                     \
      return std::get<std::vector<std::string>>(_res);                         \
    }                                                                          \
    assert(std::holds_alternative<decltype(expr)>(_res));                      \
    expr = std::move(std::get<decltype(expr)>(_res));                          \
  } while (false)

// Count code points
inline size_t ActualSize(const std::string &s) {
  return std::count_if(s.begin(), s.end(),
                       [](char ch) { return (ch & (0xc0)) != 0x80; });
}
