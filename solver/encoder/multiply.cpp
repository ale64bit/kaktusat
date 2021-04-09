#include "solver/encoder/multiply.h"

#include <stack>
#include <string>
#include <vector>

#include "solver/encoder/circuit.h"
#include "util/log.h"

namespace solver {
namespace encoder {

void Multiply(Solver &solver, int m, int n) {
  // Create the variables encoding the solution.
  std::vector<Var> x;
  std::vector<Var> y;
  std::vector<Var> z;
  for (int i = 1; i <= m; ++i) {
    x.push_back(solver.NewVar("x" + std::to_string(i)));
  }
  for (int j = 1; j <= n; ++j) {
    y.push_back(solver.NewVar("y" + std::to_string(j)));
  }
  for (int k = 1; k <= m + n; ++k) {
    z.push_back(solver.NewVar("z" + std::to_string(k)));
  }

  // Fill the bins with product bits.
  std::vector<std::stack<Var, std::vector<Var>>> bin(m + n);
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      Var pij =
          solver.NewVar("p" + std::to_string(i) + "_" + std::to_string(j));
      bin[i + j].push(pij);
      And(solver, pij, x[i], y[j]);
    }
  }

  // Process the bits in the bins.
  for (int k = 0; k < m + n; ++k) {
    while (!bin[k].empty()) {
      if (bin[k].size() == 1) {
        auto b1 = bin[k].top();
        bin[k].pop();
        Set(solver, z[k], b1);
      } else if (bin[k].size() == 2) {
        auto b1 = bin[k].top();
        bin[k].pop();
        auto b2 = bin[k].top();
        bin[k].pop();
        auto c = solver.NewTempVar("c");
        HalfAdder(solver, z[k], c, b1, b2);
        bin[k + 1].push(c);
      } else {
        auto b1 = bin[k].top();
        bin[k].pop();
        auto b2 = bin[k].top();
        bin[k].pop();
        auto b3 = bin[k].top();
        bin[k].pop();
        auto r = solver.NewTempVar("r");
        auto c = solver.NewTempVar("c");
        FullAdder(solver, r, c, b1, b2, b3);
        bin[k].push(r);
        bin[k + 1].push(c);
      }
    }
  }
}

} // namespace encoder
} // namespace solver
