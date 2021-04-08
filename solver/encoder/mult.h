#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Instance encoder for integer multiplication.
 *
 * The generated instance encodes the multiplication of an m-bit by an n-bit
 * integer resulting in a (m+n)-bit integer product. The variables are:
 *   xi : 1 <= i <= m, bits if the first factor.
 *   yi : 1 <= i <= n, bits of the second factor.
 *   zi : 1 <= i <= m+n, bits of the product.
 *
 * Gate usage = O(m*n)
 *   m*n-m-n full-adders (usually with cost = 5 gates)
 *   m half-adders (usually with cost = 2 gates)
 *   m*n additional AND-gates
 *
 * @see: 7.2.2.2 - p9
 * @see: 7.2.2.2 - exercise 41, p136
 */
void Multiplication(Solver &, int m, int n);

} // namespace encoder
} // namespace solver
