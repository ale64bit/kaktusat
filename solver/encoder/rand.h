#pragma once

#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Generates random k-SAT instances on n variables and m clauses.
 *
 * Variables and clauses are sampled with replacement.
 */
void Rand(Solver &, int n, int m, int k);

} // namespace encoder
} // namespace solver
