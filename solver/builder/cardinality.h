#pragma once

#include <vector>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Constraints given literals so that exactly one must be true.
 *
 * The input literals must be on variables previously created by other means.
 *
 * Resets solver: NO
 * Number of variables: n
 * Number of clauses: m+O(k^2) where k is the number of given literals.
 */
void ExactlyOne(Solver &, const std::vector<Lit> &);

// TODO doc
void AtLeastOne(Solver &, const std::vector<Lit> &);

// TODO doc
void AtMostOneSimple(Solver &, const std::vector<Lit> &);

// TODO doc
void AtMostOne(Solver &, Lit, Lit);
void AtMostOne(Solver &, Lit, Lit, Lit);
void AtMostOne(Solver &, Lit, Lit, Lit, Lit);
void AtMostOne(Solver &, const std::vector<Lit> &);

} // namespace builder
} // namespace solver
