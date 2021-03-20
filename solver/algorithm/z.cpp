#include "solver/algorithm/z.h"

#include <algorithm>
#include <functional>

namespace solver {
namespace algorithm {

// The slowest solver in the west.
std::pair<Result, std::vector<Lit>> Z::Solve() {
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

} // namespace algorithm
} // namespace solver
