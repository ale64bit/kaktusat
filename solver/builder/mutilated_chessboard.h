#pragma once

#include "solver/builder/builder.h"
#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for mutilated chessboard problem of size n.
 *
 * Always unsatisfiable. Useful for refutation experiments.
 *
 * @see: https://en.wikipedia.org/wiki/Mutilated_chessboard_problem
 */
void MutilatedChessboard(Solver &, int n);

} // namespace builder
} // namespace solver
