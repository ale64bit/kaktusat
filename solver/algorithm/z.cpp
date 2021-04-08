#include "solver/algorithm/z.h"

#include <algorithm>
#include <functional>

namespace solver {
namespace algorithm {

std::pair<Result, Assignment> Z::Solve() {
  std::vector<Lit> cur;
  cur.reserve(NumVars());

  std::function<bool(int)> Search = [&](int i) {
    if (i == 0) {
      return Verify(cur);
    }
    Var x(i);
    for (auto lit : {Lit(x), Lit(~x)}) {
      cur.push_back(lit);
      if (Search(i - 1)) {
        return true;
      }
      cur.pop_back();
    }
    return false;
  };

  if (Search(n_)) {
    return {Result::kSAT, cur};
  }
  return {Result::kUNSAT, {}};
}

std::pair<Result, std::vector<Assignment>> Z::SolveAll() {
  std::vector<Assignment> all;
  std::vector<Lit> cur;
  cur.reserve(NumVars());

  std::function<void(int)> Search = [&](int i) {
    if (i == 0) {
      if (Verify(cur)) {
        all.push_back(cur);
        LOG << "solution = [" << ToString(cur) << "]";
      }
      return;
    }
    Var x(i);
    for (auto lit : {Lit(x), Lit(~x)}) {
      cur.push_back(lit);
      Search(i - 1);
      cur.pop_back();
    }
  };

  Search(n_);

  if (all.empty()) {
    return {Result::kUNSAT, {}};
  }

  return {Result::kSAT, all};
}

} // namespace algorithm
} // namespace solver
