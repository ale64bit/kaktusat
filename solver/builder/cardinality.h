#pragma once

#include <vector>

#include "solver/solver.h"

namespace solver {
namespace builder {

void ExactlyOne(Solver &, const std::vector<Lit> &);

} // namespace builder
} // namespace solver
