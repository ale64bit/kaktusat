#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

template <typename T>
using Result = std::variant<std::unique_ptr<T>, std::vector<std::string>>;

template <typename T> bool ResultOK(const Result<T> &res) {
  return std::holds_alternative<std::unique_ptr<T>>(res);
}
