#pragma once

#include "solver/encoder/encoder.h"
#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Instance encoder for an impossible partial order on m elements.
 *
 * Always unsatisfiable. Useful for refutation experiments.
 *
 * @see: 7.2.2.2 - (99), (100), (101) - p56
 */
void ImpossiblePartialOrder(Solver &, int);

} // namespace encoder
} // namespace solver
