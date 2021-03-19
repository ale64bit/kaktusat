#pragma once

#include "solver/solver.h"

#include <vector>

namespace solver {
namespace builder {

void ExactlyOne(Solver &, const std::vector<Lit> &);

} // namespace builder
} // namespace solver
