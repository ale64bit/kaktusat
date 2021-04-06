#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Rivest's sample clauses. Unsatisfiable.
 *
 * @see: 7.2.2.2 - (6), p4
 */
void R(Solver &);

/*
 * Rivest's sample clauses. Satisfiable.
 *
 * @see: 7.2.2.2 - (7), p4
 */
void Rprime(Solver &);

} // namespace encoder
} // namespace solver
