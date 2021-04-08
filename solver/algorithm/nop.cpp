#include "solver/algorithm/nop.h"

namespace solver {
namespace algorithm {

std::pair<Result, Assignment> Nop::Solve() { return {Result::kUnknown, {}}; }

std::pair<Result, std::vector<Assignment>> Nop::SolveAll() {
  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
