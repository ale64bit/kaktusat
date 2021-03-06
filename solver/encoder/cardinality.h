#pragma once

#include <vector>

#include "solver/encoder/encoder.h"
#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Constraints given literals so that at least one can be true.
 *
 * The input literals must be on existing variables.
 */
void AtLeastOne(Solver &, const std::vector<Lit> &,
                Mode = Mode::kLessVariables);

/*
 * Constraints given literals so that at most one can be true.
 *
 * The input literals must be on existing variables.
 */
void AtMostOne(Solver &, const std::vector<Lit> &, Mode = Mode::kLessVariables);

/*
 * Constraints given literals so that exactly one must be true.
 *
 * The input literals must be on existing variables.
 *
 * Number of variables: n
 * Number of clauses: m+O(k^2) where k is the number of given literals.
 */
void ExactlyOne(Solver &, const std::vector<Lit> &,
                Mode = Mode::kLessVariables);

/*
 * Constraints given literals so that at least r of them can be true.
 *
 * The input literals must be on existing variables. The generated clauses
 * contain at most 3 literals.
 */
void AtLeast(Solver &, const std::vector<Lit> &, int r);

/*
 * Constraints given literals so that at most r of them can be true.
 *
 * The input literals must be on existing variables. The generated clauses
 * contain at most 3 literals.
 */
void AtMost(Solver &, const std::vector<Lit> &, int r);

/*
 * Constraints given literals so that exactly r must be true.
 *
 * The input literals must be on existing variables.
 */
void Exactly(Solver &, const std::vector<Lit> &, int r);

} // namespace encoder
} // namespace solver
