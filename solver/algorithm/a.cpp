#include "solver/algorithm/a.h"

#include <algorithm>
#include <iomanip>

#include "util/check.h"
#include "util/log.h"

namespace solver {
namespace algorithm {

std::pair<Result, std::vector<Lit>> A::Solve() {
  const int n = NumVars();

  std::vector<int> L(2 * n + 2, 0);
  std::vector<int> F(2 * n + 2, 0);
  std::vector<int> B(2 * n + 2, 0);
  std::vector<int> C(2 * n + 2, 0);
  std::vector<int> START(NumClauses() + 1, 0);
  std::vector<int> SIZE(NumClauses() + 1, 0);

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

  int a = NumClauses(); // number of active clauses.
  int d = 1;            // depth-plus-one in an implicit search tree.
  int l;                // chosen literal.

A2: // Choose.
  CHECK("depth must be 1 <= d <= n, so that we can turn it into a literal",
        1 <= d && d <= n);
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
  for (int p = F[l ^ 1]; p > 2 * n + 1; p = F[p]) {
    CHECK("every visited cell must be a non-special cell", p > 2 * n + 1);
    int j = C[p];
    if (SIZE[j] == 1) { // Makes a clause empty, so we need to restore it.
      for (int q = B[p]; q > 2 * n + 1; q = B[q]) {
        j = C[q];
        LOG << "A3: unremove " << ToString(Lit(l ^ 1)) << " from clause " << j;
        ++SIZE[j];
      }
      goto A5;
    }
    LOG << "A3: remove " << ToString(Lit(l ^ 1)) << " from clause " << j;
    --SIZE[j];
  }

A4: // Deactivate l's clauses.
  for (int p = F[l]; p > 2 * n + 1; p = F[p]) {
    int j = C[p];
    LOG << "A4: deactivate clause " << j;
    for (int i = 0; i < SIZE[j] - 1; ++i) {
      int s = START[j] + i;
      CHECK("updated counts cannot refer to the chosen literal", L[s] != l);
      // Remove the literal cell.
      // ... r=B[s] <- s -> q=F[s] ...
      int r = B[s];
      int q = F[s];
      B[q] = r;
      F[r] = q;
      // Update the active clause count for the removed literal.
      --C[L[s]];
    }
  }
  a -= C[l];
  ++d;
  goto A2;

A5: // Try again.
  if (m[d] < 2) {
    m[d] = 3 - m[d];
    l = 2 * d + (m[d] & 1);
    LOG << "A5: try again: l=" << ToString(Lit(l)) << '\n';
    goto A3;
  }

A6: // Backtrack.
  if (d == 1) {
    return {Result::kUNSAT, {}};
  } else {
    --d;
    l = 2 * d + (m[d] & 1);
    LOG << "A6: backtrack: l=" << ToString(Lit(l)) << '\n';
  }

A7: // Reactivate l's clauses.
  a += C[l];
  // Note that Knuth goes in B[l] order here, but I assume it's just for
  // symmetry.
  for (int p = F[l]; p > 2 * n + 1; p = F[p]) {
    int j = C[p];
    LOG << "A7: reactivate clause " << j;
    for (int i = 0; i < SIZE[j] - 1; ++i) {
      int s = START[j] + i;
      CHECK("updated counts cannot refer to the chosen literal", L[s] != l);
      // Restore the literal cell.
      // ... r=B[s] <- s -> q=F[s] ...
      int r = B[s];
      int q = F[s];
      F[r] = B[q] = s;
      // Update the active clause count for the restored literal.
      ++C[L[s]];
    }
  }

A8: // Unremove ~l.
  for (int p = F[l ^ 1]; p > 2 * n + 1; p = F[p]) {
    CHECK("every visited cell must be a non-special cell", p > 2 * n + 1);
    int j = C[p];
    LOG << "A8: unremove " << ToString(Lit(l ^ 1)) << " from clause " << j;
    ++SIZE[j];
    CHECK("last literal must match the unremoved literal",
          L[START[j] + SIZE[j] - 1] == (l ^ 1));
  }
  goto A5;
}

} // namespace algorithm
} // namespace solver
