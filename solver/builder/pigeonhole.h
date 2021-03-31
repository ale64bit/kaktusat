#pragma once

#include "solver/builder/builder.h"
#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for pigeonhole principle for m+1 elements.
 *
 * Always unsatisfiable. Useful for refutation experiments.
 *
 * @see: 7.2.2.2 - (106), (107) - p57
 */
void Pigeonhole(Solver &, int m);

} // namespace builder
} // namespace solver
