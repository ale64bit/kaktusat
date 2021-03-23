#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

void Unit(Solver &, std::string = "x");
void Tautology(Solver &, std::string = "x");
void Contradiction(Solver &, std::string = "x");
void R(Solver &);
void Rprime(Solver &);

} // namespace builder
} // namespace solver
