#pragma once

#include "solver/encoder/encoder.h"
#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Instance encoder for pigeonhole principle for m+1 elements.
 *
 * Always unsatisfiable. Useful for refutation experiments.
 *
 * @see: https://en.wikipedia.org/wiki/Pigeonhole_principle
 * @see: 7.2.2.2 - (106), (107) - p57
 */
void Pigeonhole(Solver &, int m);

} // namespace encoder
} // namespace solver
