#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Equivalence.
 *
 *   t = u
 *
 * @see: 7.2.2.2 - p9
 */
void Eq(Solver &, Lit t, Lit u);

/*
 * AND gate.
 *
 *   t = u & v
 *
 * @see: 7.2.2.2 - p9
 */
void And(Solver &, Lit t, Lit u, Lit v);

void And(Solver &, Lit t, const std::vector<Lit> &us);

/*
 * OR gate.
 *
 *   t = u | v
 *
 * @see: 7.2.2.2 - p9
 */
void Or(Solver &, Lit t, Lit u, Lit v);

void Or(Solver &, Lit t, const std::vector<Lit> &us);

/*
 * XOR gate.
 *
 *   t = u ^ v
 *
 * @see: 7.2.2.2 - p9
 */
void Xor(Solver &, Lit t, Lit u, Lit v);

/*
 * Half-adder.
 *
 *   t = u ^ v
 *   c = u & v
 *
 * @see: 7.2.2.2 - p9
 */
void HalfAdder(Solver &, Lit t, Lit c, Lit u, Lit v);

/*
 * Full-adder.
 *
 *   t  = u ^ v ^ c0
 *   c1 = median(u, v, c0)
 *
 * @see: 7.2.2.2 - p9
 * @see: 7.2.2.2 - exercise 42, p136
 */
void FullAdder(Solver &, Lit t, Lit c1, Lit u, Lit v, Lit c0);

} // namespace encoder
} // namespace solver
