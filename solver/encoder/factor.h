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
 * Gate usage = O(m*n)
 *   m*n-m-n full-adders (usually with cost = 5 gates)
 *   m half-adders (usually with cost = 2 gates)
 *   m*n additional AND-gates
 *
 * @see: 7.2.2.2 - p9
 * @see: 7.2.2.2 - exercise 41, p136
 */
void Factor(Solver &, int m, int n, uint64_t z);

void Factor(Solver &, int m, int n, std::vector<int> z);

} // namespace encoder
} // namespace solver
