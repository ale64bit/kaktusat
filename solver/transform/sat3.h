#pragma once

#include "solver/solver.h"

namespace solver {
namespace transform {

/*
 * Transforms the given instance into a 3SAT instance.
 *
 * In the resulting instance, each clause has at most 3 literals.
 *
 * @see: 7.2.2.2 - exercise 28, p135
 */
void SAT3(Solver &);

} // namespace transform
} // namespace solver
