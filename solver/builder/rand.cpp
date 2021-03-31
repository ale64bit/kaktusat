#include "solver/builder/rand.h"

#include <random>
#include <set>
#include <string>
#include <vector>

namespace solver {
namespace builder {

// Something simple for now, for testing purposes.
void Rand(Solver &solver, int n, int m, int k) {
  // Setup rng.
  std::random_device rdev;
  std::mt19937 gen(rdev());
  std::uniform_int_distribution<> varDist(0, n - 1);
  std::bernoulli_distribution signDist(0.5);

  // Create variables.
  std::vector<Var> x;
  for (int i = 1; i <= n; ++i) {
    x.push_back(solver.NewVar(std::to_string(i)));
  }

  // Create clauses.
  for (int i = 0; i < m; ++i) {
    std::set<int> used;
    do {
      used.insert(varDist(gen));
    } while (used.size() < k);
    std::vector<Lit> clause;
    for (auto j : used) {
      clause.push_back(signDist(gen) ? x[j] : ~x[j]);
    }
    solver.AddClause(clause);
  }
}

} // namespace builder
} // namespace solver
