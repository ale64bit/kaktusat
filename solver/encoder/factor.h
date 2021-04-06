#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Instance encoder for integer factorization.
 *
 * The generated instance is satisfiable when z can be factored into two factors
 * of m and n bits, respectively.
 *
 * @see: 7.2.2.2 - p9
 */
void Factor(Solver &, int m, int n, uint64_t z);

void Factor(Solver &, int m, int n, std::vector<int> z);

} // namespace encoder
} // namespace solver
