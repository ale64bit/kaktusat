#include "solver/builder/cardinality.h"

#include <iostream>

namespace solver {
namespace builder {

void ExactlyOne(Solver &solver, const std::vector<Lit> &y, Mode mode) {
  AtLeastOne(solver, y, mode);
  AtMostOne(solver, y, mode);
}

void AtLeastOne(Solver &solver, const std::vector<Lit> &y, Mode) {
  solver.AddClause(y);
}

/*
 * Simple implementation: for each pair x and y, one of them must be false.
 *
 * It doesn't use new variables, but generates O(|y|^2) binary clauses.
 *
 * @see: 7.2.2.2 - p6
 */
static void AtMostOneSimple(Solver &solver, const std::vector<Lit> &y) {
  for (size_t i = 0; i < y.size(); ++i) {
    for (size_t j = i + 1; j < y.size(); ++j) {
      solver.AddClause({~y[i], ~y[j]});
    }
  }
}

/*
 * Generates less clauses, at the expense of more variables.
 *
 * Produces floor((p-3)/2) new variables and 3*p-5 clauses, where p=|y|.
 *
 * @see: 7.2.2.2 - p6
 * @see: 7.2.2.2 - exercise 12, p134
 */
static void AtMostOneLessClauses(Solver &solver, const std::vector<Lit> &y) {
  if (y.size() <= 4) {
    AtMostOneSimple(solver, y);
    return;
  }
  auto t1 = solver.NewTempVar();
  AtMostOneSimple(solver, {y[0], y[1], y[2], t1});
  for (size_t i = 3; i < y.size(); i += 2) {
    if (i == y.size() - 1) {
      // 1 remaining
      AtMostOneSimple(solver, {~t1, y[i]});
      break;
    } else if (i == y.size() - 2) {
      // 2 remaining
      AtMostOneSimple(solver, {~t1, y[i], y[i + 1]});
      break;
    } else if (i == y.size() - 3) {
      // 3 remaining
      AtMostOneSimple(solver, {~t1, y[i], y[i + 1], y[i + 2]});
      break;
    } else {
      // >3 remaining
      auto t2 = solver.NewTempVar();
      AtMostOneSimple(solver, {~t1, y[i], y[i + 1], t2});
      t1 = t2;
    }
  }
}

void AtMostOne(Solver &solver, const std::vector<Lit> &y, Mode mode) {
  switch (mode) {
  case Mode::kLessVariables:
    AtMostOneSimple(solver, y);
    break;
  case Mode::kLessClauses:
    AtMostOneLessClauses(solver, y);
    break;
  }
}

void AtLeast(Solver &solver, const std::vector<Lit> &x, int r) {
  const int n = (int)x.size();
  std::vector<Lit> y;
  for (const auto &xi : x) {
    y.push_back(~xi);
  }
  return AtMost(solver, y, n - r);
}

// Sinz's method.
void AtMostMethod1(Solver &solver, const std::vector<Lit> &x, int r) {
  const int n = (int)x.size();

  // Create additional variables.

  // s(k,j) : 1<=k<=r, 1<=j<=n-r
  std::vector<std::vector<Var>> s(r);
  for (int k = 1; k <= r; ++k) {
    for (int j = 1; j <= n - r; ++j) {
      std::string id = "s" + std::to_string(k) + "_" + std::to_string(j);
      s[k - 1].push_back(solver.NewTempVar(id));
    }
  }

  // Add constraints.

  // (~s(k,j) \/ s(k,j+1))
  for (int k = 1; k <= r; ++k) {
    for (int j = 1; j < n - r; ++j) {
      solver.AddClause({~s[k - 1][j - 1], s[k - 1][j]});
    }
  }

  // (~x(j+k) \/ ~s(k,j) \/ s(k+1,j))
  for (int k = 0; k <= r; ++k) {
    for (int j = 1; j <= n - r; ++j) {
      std::vector<Lit> clause{~x[j + k - 1]};
      if (k > 0) {
        clause.emplace_back(~s[k - 1][j - 1]);
      }
      if (k < r) {
        clause.emplace_back(s[k][j - 1]);
      }
      solver.AddClause(clause);
    }
  }
}

void AtMost(Solver &solver, const std::vector<Lit> &x, int r) {
  AtMostMethod1(solver, x, r);
}

} // namespace builder
} // namespace solver
