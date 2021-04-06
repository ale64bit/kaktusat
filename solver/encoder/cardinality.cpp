#include "solver/encoder/cardinality.h"

#include "util/log.h"

namespace solver {
namespace encoder {

void AtLeastOne(Solver &solver, const std::vector<Lit> &y, Mode) {
  CHECK(!y.empty()) << "there must be at least one literal";
  solver.AddClause(y);
}

/*
 * Simple implementation: for each pair x and y, one of them must be false.
 *
 * It doesn't use additional variables, but generates O(|y|^2) binary clauses.
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

void ExactlyOne(Solver &solver, const std::vector<Lit> &y, Mode mode) {
  AtLeastOne(solver, y, mode);
  AtMostOne(solver, y, mode);
}

void AtLeast(Solver &solver, const std::vector<Lit> &x, int r) {
  CHECK(r <= (int)x.size())
      << "there must be at least r=" << r << " literals, got |x|=" << x.size();
  const int n = (int)x.size();
  std::vector<Lit> y;
  for (const auto &xi : x) {
    y.push_back(~xi);
  }
  return AtMost(solver, y, n - r);
}

/*
 * Sinz C. (2005) Towards an Optimal CNF Encoding of Boolean Cardinality
 * Constraints. In: van Beek P. (eds) Principles and Practice of Constraint
 * Programming - CP 2005. CP 2005. Lecture Notes in Computer Science, vol 3709.
 * Springer, Berlin, Heidelberg. https://doi.org/10.1007/11564751_73
 *
 * @see: 7.2.2.2 - p8
 * @see: 7.2.2.2 - exercise 26, p135
 */
void AtMostMethod1(Solver &solver, const std::vector<Lit> &x, int r) {
  const int n = (int)x.size();

  // Create additional variables.

  // s(k,j) : 1 <= k <= r, 1 <= j <= n-r
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

/*
 * Bailleux O., Boufkhad Y. (2003) Efficient CNF Encoding of Boolean Cardinality
 * Constraints. In: Rossi F. (eds) Principles and Practice of Constraint
 * Programming – CP 2003. CP 2003. Lecture Notes in Computer Science, vol 2833.
 * Springer, Berlin, Heidelberg. https://doi.org/10.1007/978-3-540-45193-8_8
 *
 * @see: 7.2.2.2 - p8
 * @see: 7.2.2.2 - exercise 27, p135
 */
void AtMostMethod2(Solver &solver, const std::vector<Lit> &x, int r) {
  const int n = (int)x.size();

  // Calculate leaf counts.
  //   * internal nodes are 1..n-1
  //   * leaves are n..2n-1
  std::vector<int> t(2 * n);

  for (int k = n; k < 2 * n; ++k) {
    t[k] = 1;
  }
  for (int k = n - 1; k >= 1; --k) {
    t[k] = std::min(r, t[2 * k] + t[2 * k + 1]);
  }

  // Create new variables.
  std::vector<std::vector<Lit>> b(2 * n);
  for (int k = 2; k < 2 * n; ++k) {
    for (int j = 1; j <= t[k]; ++j) {
      if (k < n) {
        b[k].push_back(solver.NewTempVar("b" + std::to_string(j) + "_" +
                                         std::to_string(k)));
      } else {
        b[k].push_back(x[k - n]);
      }
    }
  }

  // Add constraints on internal nodes.
  for (int k = 2; k < n; ++k) {
    for (int i = 0; i <= t[2 * k]; ++i) {
      for (int j = 0; j <= t[2 * k + 1] && i + j <= t[k] + 1; ++j) {
        if (i == 0 && j == 0)
          continue;

        std::vector<Lit> clause;
        if (1 <= i + j && i + j <= t[k])
          clause.emplace_back(b[k][i + j - 1]);
        if (1 <= i && i <= t[2 * k])
          clause.emplace_back(~b[2 * k][i - 1]);
        if (1 <= j && j <= t[2 * k + 1])
          clause.emplace_back(~b[2 * k + 1][j - 1]);
        solver.AddClause(clause);
      }
    }
  }

  // Add boundary conditions.
  for (int i = 0; i <= t[2]; ++i) {
    int j = r + 1 - i;
    if (0 <= j && j <= t[3]) {
      std::vector<Lit> clause;
      if (1 <= i && i <= t[2])
        clause.emplace_back(~b[2][i - 1]);
      if (1 <= j && j <= t[3])
        clause.emplace_back(~b[3][j - 1]);
      solver.AddClause(clause);
    }
  }
}

void AtMost(Solver &solver, const std::vector<Lit> &x, int r) {
  CHECK(r <= (int)x.size())
      << "there must be at least r=" << r << " literals, got |x|=" << x.size();
  // AtMostMethod1(solver, x, r);
  AtMostMethod2(solver, x, r);
}

void Exactly(Solver &solver, const std::vector<Lit> &x, int r) {
  CHECK(r <= (int)x.size())
      << "there must be at least r=" << r << " literals, got |x|=" << x.size();
  AtLeast(solver, x, r);
  AtMost(solver, x, r);
}

} // namespace encoder
} // namespace solver
