#include "solver/algorithm/A.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "util/check.h"

namespace solver {
namespace algorithm {

// 7.2.2.2 - Algorithm A - p28
std::pair<Result, std::vector<Lit>> A::Solve() {
  const int n = NumVars();

  std::vector<int> L(2 * n + 2, 0);
  std::vector<int> F(2 * n + 2, 0);
  std::vector<int> B(2 * n + 2, 0);
  std::vector<int> C(2 * n + 2, 0);
  std::vector<int> START(NumClauses() + 1, 0);
  std::vector<int> SIZE(NumClauses() + 1, 0);
  // TODO: is ACTIVE actually needed?
  std::vector<bool> ACTIVE(NumClauses() + 1, true);

  // 2 .. 2n+1
  // F[l] = forward, first cell that contains literal l
  // B[l] = backward, last cell that contains literal l
  // C[l] = number of currently active cells in which l appears

  // 2n+2 .. inf
  // L[p] = literal contained in this cell
  // F[p] = forward, next cell that contains literal L[p]
  // B[p] = backward, prev cell that contains literal L[p]
  // C[p] = clause in which literal L[p] appears

  // ================================================================================
  // Build initial structure.
  int p = 2 * n + 2;
  for (int i = NumClauses() - 1; i >= 0; --i) {
    std::sort(clauses_[i].rbegin(), clauses_[i].rend());
    START[i + 1] = p;
    SIZE[i + 1] = static_cast<int>(clauses_[i].size());
    for (Lit ll : clauses_[i]) {
      const int l = ll.ID();
      L.push_back(l);
      F.push_back(F[l] == 0 ? l : F[l]);
      F[l] = p;
      B.push_back(l);
      B[F.back()] = p;
      C.push_back(i + 1);
      C[l]++;
      ++p;
    }
  }

  // m[j] = 0: trying xj, didn't try ~xj yet.
  // m[j] = 1: trying ~xj, didn't try xj, yet.
  // m[j] = 2: trying xj, after ~xj failed.
  // m[j] = 3: trying ~xj, after xj failed.
  // m[j] = 4: trying xj, and ~xj doesn't appear.
  // m[j] = 5: trying ~xj, and xj doesn't appear.
  std::vector<int> m(n + 1, 0);

A1: // Initialize.

  int a = NumClauses();  // number of active clauses.
  int d = 1;             // depth-plus-one in an implicit search tree.
  int l;                 // chosen literal.
  bool makesClauseEmpty; // whether selecting l makes a clause empty.

A2: // Choose.
  CHECK("Depth must be 1 <= d <= n, so that we can turn it into a literal",
        1 <= d && d <= n);
  l = 2 * d;
  if (C[l] <= C[l + 1]) {
    ++l;
  }
  m[d] = (l & 1) + 4 * (C[l ^ 1] == 0);

  if (C[l] == a) {
    std::vector<Lit> ret;
    for (int j = 1; j <= d; ++j) {
      Var x(j);
      ret.push_back((1 ^ (m[j] & 1)) ? x : ~x);
    }
    return {Result::kSAT, ret};
  }

A3: // Remove ~l.
  makesClauseEmpty = false;
  for (int p = F[l ^ 1]; p > 2 * n + 1; p = F[p]) {
    if (ACTIVE[C[p]] && SIZE[C[p]] == 1) {
      CHECK("The only literal left is the one we want to remove",
            L[START[C[p]]] == (l ^ 1));
      makesClauseEmpty = true;
      break;
    }
  }
  if (makesClauseEmpty) {
    goto A5;
  }
  for (int p = F[l ^ 1]; p > 2 * n + 1; p = F[p]) {
    CHECK("Every visited cell must be a non-special cell", p > 2 * n + 1);
    if (ACTIVE[C[p]]) {
      CHECK("Active clauses cannot be empty", SIZE[C[p]] > 0);
      CHECK("The last literal must match the literal being removed",
            L[START[C[p]] + SIZE[C[p]] - 1] == (l ^ 1));
      --SIZE[C[p]];
      CHECK("The resulting clause cannot be empty", SIZE[C[p]] > 0);
    }
  }

A4: // Deactivate l's clauses.
  for (int p = F[l]; p > 2 * n + 1; p = F[p]) {
    if (ACTIVE[C[p]]) {
      CHECK("The chosen literal must be the last one in the clauses to be "
            "deactivated",
            L[START[C[p]] + SIZE[C[p]] - 1] == l);
      ACTIVE[C[p]] = false;
      for (int j = 0; j < SIZE[C[p]] - 1; ++j) {
        CHECK("Updated counts cannot refer to the chosen literal",
              L[START[C[p]] + j] != l);
        --C[L[START[C[p]] + j]];
      }
    }
  }
  a -= C[l];
  ++d;
  goto A2;

A5: // Try again.
  if (m[d] < 2) {
    m[d] = 3 - m[d];
    l = 2 * d + (m[d] & 1);
    goto A3;
  }

A6: // Backtrack.
  if (d == 1) {
    return {Result::kUNSAT, {}};
  } else {
    --d;
    l = 2 * d + (m[d] & 1);
  }

A7: // Reactivate l's clauses.
  a += C[l];
  for (int p = F[l]; p > 2 * n + 1; p = F[p]) {
    if (!ACTIVE[C[p]] && L[START[C[p]] + SIZE[C[p]] - 1] == l) {
      ACTIVE[C[p]] = true;
      for (int j = 0; j < SIZE[C[p]] - 1; ++j) {
        CHECK("Updated counts cannot refer to the chosen literal",
              L[START[C[p]] + j] != l);
        ++C[L[START[C[p]] + j]];
      }
    }
  }

A8: // Unremove ~l.
  for (int p = F[l ^ 1]; p > 2 * n + 1; p = F[p]) {
    CHECK("Every visited cell must be a non-special cell", p > 2 * n + 1);
    if (ACTIVE[C[p]]) {
      ++SIZE[C[p]];
      CHECK("The literal added back must match the literal being unremoved",
            L[START[C[p]] + SIZE[C[p]] - 1] == (l ^ 1));
    }
  }
  goto A5;
}

} // namespace algorithm
} // namespace solver
