#include "solver/algorithm/nop.h"

namespace solver {
namespace algorithm {

std::pair<Result, std::vector<Lit>> Nop::Solve() {
  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
