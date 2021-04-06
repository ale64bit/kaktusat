#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Encoder mode to favor different characteristics.
 */
enum class Mode {
  /* Prefer instances with less variables. */
  kLessVariables,
  /* Prefer instances with less clauses. */
  kLessClauses,
};

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

/*
 * Creates a new variable and a contradiction (x) ∧ (¬x) associated with it.
 *
 * Number of variables: n+1
 * Number of clauses: m+2
 */
void Contradiction(Solver &, std::string = "x");

} // namespace encoder
} // namespace solver
