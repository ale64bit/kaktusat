#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Creates a new variable and a unit clause (x) associated with it.
 *
 * Resets solver: NO
 * Number of variables: n+1
 * Number of clauses: m+1
 */
void Unit(Solver &, std::string = "x");

/*
 * Creates a new variable and a tautology (x ∨ ¬x) associated with it.
 *
 * Resets solver: NO
 * Number of variables: n+1
 * Number of clauses: m+1
 */
void Tautology(Solver &, std::string = "x");

/* Creates a new variable and a contradiction (x) ∧ (¬x) associated with it.
 *
 * Resets solver: NO
 * Number of variables: n+1
 * Number of clauses: m+2
 */
void Contradiction(Solver &, std::string = "x");

/* 7.2.2.2 - (6), p4 */
void R(Solver &);

/* 7.2.2.2 - (7), p4 */
void Rprime(Solver &);

/*
 * Transforms the given instance into a monotonic instance.
 *
 * In the resulting instance, each clause has only all-positive or all-negative
 * literals. Variable names are preserved by appending _0 and _1 to the original
 * names.
 *
 * Resets solver: NO
 * Number of variables: 2n
 * Number of clauses: m+n
 *
 * 7.2.2.2 - exercise 10, p133
 */
void Monotonize(Solver &);

} // namespace builder
} // namespace solver
