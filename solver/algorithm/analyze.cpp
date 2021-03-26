#include "solver/algorithm/analyze.h"

#include <iostream>
#include <map>

namespace solver {
namespace algorithm {

std::pair<Result, std::vector<Lit>> Analyze::Solve() {
  std::map<int, int> clauseLenCnt;
  for (const auto &clause : clauses_) {
    clauseLenCnt[clause.size()]++;
  }

  std::clog << "N=" << NumVars() << " M=" << NumClauses() << '\n';
  std::clog << "Clause length counts:" << '\n';
  for (const auto &[len, cnt] : clauseLenCnt) {
    std::clog << "\t" << len << ": " << cnt << '\n';
  }

  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
