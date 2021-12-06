#pragma once

#include "solver/solver.h"

namespace solver {
namespace encoder {

// Builds a minimal implicant of size g in n variables
// TODO: extremely slow. Needs better encoding.
void MinimalImplicant(Solver &, int n, int g);

} // namespace encoder
} // namespace solver
