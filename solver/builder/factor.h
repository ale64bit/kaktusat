#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for integer factorization.
 *
 * The generated instance is satisfiable when z can be factored into two factors
 * of m and n bits, respectively.
 *
 * @see: 7.2.2.2 - p9
 */
void Factor(Solver &, int m, int n, uint64_t z);

} // namespace builder
} // namespace solver
