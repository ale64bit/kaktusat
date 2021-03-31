#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Creates a new variable and a unit clause (x) associated with it.
 *
 * Number of variables: n+1
 * Number of clauses: m+1
 */
void Unit(Solver &, std::string = "x");

/*
 * Creates a new variable and a tautology (x ∨ ¬x) associated with it.
 *
 * Number of variables: n+1
 * Number of clauses: m+1
 */
void Tautology(Solver &, std::string = "x");

/* Creates a new variable and a contradiction (x) ∧ (¬x) associated with it.
 *
 * Number of variables: n+1
 * Number of clauses: m+2
 */
void Contradiction(Solver &, std::string = "x");

/* 7.2.2.2 - (6), p4 */
void R(Solver &);

/* 7.2.2.2 - (7), p4 */
void Rprime(Solver &);

} // namespace builder
} // namespace solver
