#pragma once

#include "solver/solver.h"

namespace solver {
namespace transform {

/*
 * Transforms the given instance into a monotonic instance.
 *
 * In the resulting instance, each clause has only all-positive or all-negative
 * literals. Variable names are preserved by appending _0 and _1 to the original
 * names.
 *
 * Number of variables: 2n
 * Number of clauses: m+n
 *
 * @see: 7.2.2.2 - exercise 10, p133
 */
void Monotonic(Solver &);

} // namespace transform
} // namespace solver
