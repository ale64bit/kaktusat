#pragma once

#include "solver/builder/builder.h"
#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for an impossible partial order on m elements.
 *
 * Useful for refutation tree experiments.
 *
 * @see: 7.2.2.2 - p56
 */
void ImpossiblePartialOrder(Solver &, int);

} // namespace builder
} // namespace solver
