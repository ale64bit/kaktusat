#include "solver/encoder/factor.h"

#include <stack>
#include <string>
#include <vector>

#include "solver/encoder/multiply.h"
#include "util/log.h"

namespace solver {
namespace encoder {

void Factor(Solver &solver, int m, int n, uint64_t zz) {
  std::vector<int> z;
  for (int k = 0; k < m + n; ++k, zz >>= 1) {
    z.push_back(zz & 1);
  }
  CHECK(zz == 0) << "not enough bits to represent the product";
  return Factor(solver, m, n, z);
}

void Factor(Solver &solver, int m, int n, std::vector<int> zz) {
  if (m > n) {
    std::swap(m, n);
  }
  CHECK(m + n >= (int)zz.size())
      << "not enough bits to represent the product z, need at least "
      << zz.size() << ", got m+n=" << m + n;
  for (auto d : zz) {
    CHECK(d == 0 || d == 1) << "z must have only binary digits, got " << d;
  }

  // Encode a multiplication instance.
  Multiply(solver, m, n);

  // Get the variables corresponding to the product.
  std::vector<Lit> z;
  for (int i = 1; i <= m + n; ++i) {
    z.emplace_back(solver.GetVar("z" + std::to_string(i)));
  }

  // Add unit clauses representing zz.
  for (int k = 0; k < m + n; ++k) {
    if (zz[k]) {
      solver.AddClause({z[k]});
    } else {
      solver.AddClause({~z[k]});
    }
  }
}

} // namespace encoder
} // namespace solver
