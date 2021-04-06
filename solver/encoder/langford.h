#pragma once

#include "solver/encoder/encoder.h"
#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Instance encoder for Langford's pairs problem.
 *
 * @see: https://en.wikipedia.org/wiki/Langford_pairing
 * @see: 7.2.2.2 - (11), p5
 */
void Langford(Solver &, int, Mode = Mode::kLessVariables);

} // namespace encoder
} // namespace solver
