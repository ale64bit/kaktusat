#include "solver/algorithm/a2.h"

#include <algorithm>
#include <iomanip>

#include "util/log.h"

namespace solver {
namespace algorithm {

std::pair<Result, Assignment> A2::Solve() {
  const int n = NumVars();

  std::vector<int> L(2 * n + 2, 0);
  std::vector<int> F(2 * n + 2, 0);
  std::vector<int> C(2 * n + 2, 0);
  std::vector<int> START(NumClauses() + 1, 0);
  std::vector<int> SIZE(NumClauses() + 1, 0);

  // 2 .. 2n+1
  // F[l] = forward, first cell that contains literal l
  // C[l] = number of currently active cells in which l appears

  // 2n+2 .. inf
  // L[p] = literal contained in this cell
  // F[p] = forward, next cell that contains literal L[p]
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
      C.push_back(i + 1);
      C[l]++;
      ++p;
    }
  }

  // Move codes:
  //   m[j] = 0: trying xj, didn't try ~xj yet.
  //   m[j] = 1: trying ~xj, didn't try xj, yet.
  //   m[j] = 2: trying xj, after ~xj failed.
  //   m[j] = 3: trying ~xj, after xj failed.
  //   m[j] = 4: trying xj, and ~xj doesn't appear.
  //   m[j] = 5: trying ~xj, and xj doesn't appear.
  std::vector<int> m(n + 1, 0);

A1: // Initialize.

  int a = NumClauses();  // number of active clauses.
  int d = 1;             // depth-plus-one in an implicit search tree.
  int l;                 // chosen literal.
  bool makesClauseEmpty; // whether selecting l makes a clause empty.

  auto LastLiteral = [&](int j) {
    CHECK(1 <= j && j <= NumClauses()) << "clause index out of bounds: " << j;
    return L[START[j] + SIZE[j] - 1];
  };

A2: // Choose.
  CHECK(1 <= d && d <= n) << "depth must be 1 <= d <= n, got d=" << d;
  l = 2 * d;
  if (C[l] <= C[l + 1]) {
    ++l;
  }
  m[d] = (l & 1) + 4 * (C[l ^ 1] == 0);
  LOG << "A2: choose l=" << ToString(Lit(l)) << " a=" << a;

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
    CHECK(p > 2 * n + 1)
        << "every visited cell must be a non-special cell, got p=" << p;
    int j = C[p];
    if (LastLiteral(j) == (l ^ 1) && SIZE[j] == 1) {
      makesClauseEmpty = true;
      break;
    }
  }
  if (makesClauseEmpty) {
    goto A5;
  }
  for (int p = F[l ^ 1]; p > 2 * n + 1; p = F[p]) {
    CHECK(p > 2 * n + 1)
        << "every visited cell must be a non-special cell, got p=" << p;
    int j = C[p];
    if (LastLiteral(j) == (l ^ 1)) {
      LOG << "A3: remove " << ToString(Lit(l ^ 1)) << " from clause " << j;
      --SIZE[j];
      CHECK(SIZE[j] > 0) << "the resulting clause cannot be empty";
    }
  }

A4: // Deactivate l's clauses.
  for (int p = F[l]; p > 2 * n + 1; p = F[p]) {
    int j = C[p];
    if (LastLiteral(j) == l) {
      LOG << "A4: deactivate clause " << j;
      for (int i = 0; i < SIZE[j] - 1; ++i) {
        CHECK(L[START[j] + i] != l)
            << "updated counts cannot refer to the chosen literal";
        --C[L[START[j] + i]];
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
    LOG << "A5: try again: l=" << ToString(Lit(l));
    goto A3;
  }

A6: // Backtrack.
  if (d == 1) {
    return {Result::kUNSAT, {}};
  } else {
    --d;
    l = 2 * d + (m[d] & 1);
    LOG << "A6: backtrack: l=" << ToString(Lit(l));
  }

A7: // Reactivate l's clauses.
  a += C[l];
  for (int p = F[l]; p > 2 * n + 1; p = F[p]) {
    int j = C[p];
    if (LastLiteral(j) == l) {
      LOG << "A7: reactivate clause " << j;
      for (int i = 0; i < SIZE[j] - 1; ++i) {
        CHECK(L[START[j] + i] != l)
            << "updated counts cannot refer to the chosen literal";
        ++C[L[START[j] + i]];
      }
    }
  }

A8: // Unremove ~l.
  for (int p = F[l ^ 1]; p > 2 * n + 1; p = F[p]) {
    CHECK(p > 2 * n + 1)
        << "every visited cell must be a non-special cell, got p=" << p;
    int j = C[p];
    if (LastLiteral(j) > (l ^ 1)) {
      LOG << "A8: unremove " << ToString(Lit(l ^ 1)) << " from clause " << j;
      ++SIZE[j];
      CHECK(LastLiteral(j) == (l ^ 1))
          << "last literal must match the unremoved literal, got "
          << ToString(Lit(LastLiteral(j)));
    }
  }
  goto A5;
}

std::pair<Result, std::vector<Assignment>> A2::SolveAll() {
  COMMENT << "this solver does not support listing all satisfying assignments";
  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
