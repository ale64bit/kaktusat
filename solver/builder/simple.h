#pragma once

#include "solver/solver.h"

namespace solver {
namespace builder {

void Unit(Solver &);
void Tautology(Solver &);
void Contradiction(Solver &);
void R(Solver &);
void Rprime(Solver &);

} // namespace builder
} // namespace solver
