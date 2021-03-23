#pragma once

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for Langford's pairs problem.
 *
 * @see: 7.2.2.2 - (11), p5
 */
void Langford(Solver &, int);

} // namespace builder
} // namespace solver
