#pragma once

#include <vector>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for van der Waerden numbers on two colors.
 *
 * Waerden(j, k, n) generates an instance that is satisfiable iff n < W(j, k).
 * In other words, it computes a binary sequence of length n such that there
 * are no j equally spaced 0s and no k equally spaced 1s.
 *
 * Number of variables: n
 * Number of clauses: f(j,n) + f(k,n)
 *   where f(i,n) = floor(n/(i-1)) * (n - (i-1)*(floor(n/(i-1))+1)/2) = O(i*n^2)
 *
 * @see: https://en.wikipedia.org/wiki/Van_der_Waerden_number
 * @see: 7.2.2.2 - (10), p4
 * @see: 7.2.2.2 - exercise 3, p133
 */
void Waerden(Solver &, int, int, int);

/*
 * Instance builder for general van der Waerden numbers.
 *
 * Waerden([k_1, ..., k_b], n) generates an instance that is satisfiable iff n <
 * W(k_1, ..., k_b).
 *
 * @see: https://en.wikipedia.org/wiki/Van_der_Waerden_number
 * @see: 7.2.2.2 - (10), p4
 * @see: 7.2.2.2 - exercise 8, p133
 */
void Waerden(Solver &, std::vector<int>, int);

} // namespace builder
} // namespace solver
