#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Assignment: t = u
 *
 * @see: 7.2.2.2 - p9
 */
void Set(Solver &, Var t, Var u);

/*
 * And: t = u /\ v
 *
 * @see: 7.2.2.2 - p9
 */
void And(Solver &, Var t, Var u, Var v);

/*
 * Or: t = u \/ v
 *
 * @see: 7.2.2.2 - p9
 */
void Or(Solver &, Var t, Var u, Var v);

/*
 * Xor: t = u + v
 *
 * @see: 7.2.2.2 - p9
 */
void Xor(Solver &, Var t, Var u, Var v);

/*
 * Half-adder storing the result in t and the carry in c.
 *
 *   t = u + v
 *   c = u /\ v
 *
 * @see: 7.2.2.2 - p9
 */
void HalfAdder(Solver &, Var t, Var c, Var u, Var v);

} // namespace builder
} // namespace solver
