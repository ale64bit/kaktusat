#include <cstdlib>
#include <iostream>
#include <string>

#include "solver/algorithm/d.h"
#include "solver/encoder/encoder.h"
#include "solver/encoder/factor.h"
#include "util/log.h"

int main(int argc, char *argv[]) {
  util::InitLogging();

  if (argc != 2) {
    std::cerr << "usage: factor <n>" << std::endl;
    return 1;
  }

  // Read input integer to factorize.
  uint64_t n = std::strtoull(argv[1], nullptr, 0);
  if (n == 0) {
    std::cerr << "invalid input: '" << argv[1] << "'" << std::endl;
    return 1;
  }

  // Calculate the number of bits allowed for the factors.
  int b = 0;
  while ((1 << b) < n) {
    ++b;
  }
  b = (b + 1) / 2;
  std::cout << "trying to find factors of at most " << b << " bits"
            << std::endl;

  solver::algorithm::D solver;
  uint64_t x = 0;
  uint64_t y = 0;

  // Use the factorization encoder.
  solver::encoder::Factor(solver, b, b, n);
  std::cout << "resulting instance has " << solver.NumVars()
            << " variables and " << solver.NumClauses() << " clauses"
            << std::endl;

  // Solve the instance.
  auto [res, sol] = solver.Solve();
  switch (res) {
  case solver::Result::kSAT:
    // Decode the factors from the solution literals.
    for (const auto &l : sol) {
      if (l.IsPos() && !solver.IsTemp(l.V())) {
        std::string name = solver.NameOf(l.V());
        if (name[0] == 'x') {
          int idx = std::atoi(name.data() + 1);
          x ^= (1ull << (idx - 1));
        } else if (name[0] == 'y') {
          int idx = std::atoi(name.data() + 1);
          y ^= (1ull << (idx - 1));
        }
      }
    }
    std::cout << n << " = " << x << " * " << y << std::endl;
    break;
  case solver::Result::kUNSAT:
    std::cout << "no factorization found in " << b << "-bit factors"
              << std::endl;
    break;
  case solver::Result::kUnknown:
    std::cout << "instance could not be solved" << std::endl;
    break;
  }

  return 0;
}
