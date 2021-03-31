#pragma once

#include "solver/builder/builder.h"
#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for Langford's pairs problem.
 *
 * @see: https://en.wikipedia.org/wiki/Langford_pairing
 * @see: 7.2.2.2 - (11), p5
 */
void Langford(Solver &, int, Mode = Mode::kLessVariables);

} // namespace builder
} // namespace solver
