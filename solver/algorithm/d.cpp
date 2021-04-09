#include "solver/algorithm/d.h"

#include <algorithm>
#include <iomanip>

#include "util/log.h"

namespace solver {
namespace algorithm {

std::pair<Result, Assignment> D::Solve() {
  // L[i]     = i-th cell's literal.
  // W[l]     = first clause watching literal l or 0 if none.
  // START[j] = first cell of clause j
  // LINK[j]  = next clause watching the same literal as clause j, or 0 if none.
  std::vector<int> L{0};
  std::vector<int> W(2 * NumVars() + 2, 0);
  std::vector<int> START(NumClauses() + 1, 0);
  std::vector<int> LINK(NumClauses() + 1, 0);

  // Active ring of variables.
  //
  // head       = first element of the ring.
  // tail       = last element of the ring.
  // NEXT[j]    = next element of the ring after variable j.
  // NEXT[tail] = head
  int head;
  int tail;
  std::vector<int> NEXT(NumVars() + 1, 0);

  // x[j] = 0 if j is set to fals, 1 otherwise.
  // h[j] = choice at level j.
  std::vector<int> x(NumVars() + 1, 0);
  std::vector<int> h(NumVars() + 1, 0);

  // Move codes:
  //   m[j] = 0: trying x[h[j]], didn't try ~x[h[j]] yet.
  //   m[j] = 1: trying ~x[h[j]], didn't try x[h[j]] yet.
  //   m[j] = 2: trying x[h[j]], after ~x[h[j]] failed.
  //   m[j] = 3: trying ~x[h[j]], after x[h[j]] failed.
  //   m[j] = 4: trying x[h[j]], forced by unit clause.
  //   m[j] = 5: trying ~x[h[j]], forced by unit clause.
  std::vector<int> m(NumVars() + 1, 0);

  // Build the clause data structure and watch lists.
  // Literals of clause j are in the cells START[j] to START[j-1]-1.
  for (int j = NumClauses(); j >= 1; --j) {
    START[j] = L.size();
    for (auto l : clauses_[j - 1]) {
      L.push_back(l.ID());
    }
    int l = L[START[j]]; // this clause's watchee.
    LINK[j] = W[l];
    W[l] = j;
  }
  START[0] = L.size();

D1: // Initialize.

  int d = 0;              // depth of implicit search tree.
  int k;                  // current variable.
  int f;                  // whether a literal is unit.
  int branchScore = 0;    // score used to select the best branching variable.
  int bestBranchTail = 0; // tail to the best branching variable in the ring.
  m[0] = 0;
  head = 0;
  tail = 0;
  for (int i = NumVars(); i >= 1; --i) {
    x[i] = -1;
    // If a variable is watched, add it to the ring.
    if ((W[2 * i] != 0) || (W[2 * i + 1] != 0)) {
      LOG << "D1: variable " << ToString(Var(i)) << " added to ring";
      NEXT[i] = head;
      head = i;
      if (tail == 0) {
        tail = i;
      }
    }
  }
  if (tail != 0) {
    NEXT[tail] = head;
  }

D2: // Success?

  // If the ring is empty, no variable is being watched.
  if (tail == 0) {
    std::vector<Lit> ret;
    for (int j = 1; j <= d; ++j) {
      CHECK(x[h[j]] != -1) << "each solution literal must have a known value";
      Var y(h[j]);
      ret.push_back(x[h[j]] ? y : ~y);
    }
    return {Result::kSAT, ret};
  }
  k = tail;
  branchScore = 0;

D3: // Look for unit clauses.
  head = NEXT[k];
  CHECK(1 <= head && head <= NumVars())
      << "head must be a valid variable, got head=" << head;
  // Check if variable k is watched by some unit clause.
  f = 0;
  int len = 0; // len of the current watchlist.
  for (int l : {2 * head + 1, 2 * head}) {
    f *= 2;
    for (int j = W[l]; j != 0; j = LINK[j]) {
      CHECK(L[START[j]] == l)
          << "clause j=" << j << " should be watching " << ToString(Lit(l))
          << " but it's watching " << ToString(Lit(L[START[j]]));
      // Check if clause j is unit.
      bool unit = true;
      for (int i = START[j] + 1; i < START[j - 1]; ++i) {
        // Check whether L[i] is not set or already set to true.
        int xi = L[i] >> 1;
        if (x[xi] == -1 || x[xi] == (1 ^ (L[i] & 1))) {
          unit = false;
          break;
        }
      }
      if (unit) {
        f += 1;
        break;
      }
      ++len;
    }
  }
  // Select the branching variable with the largest watched literal count.
  // @see: 7.2.2.2 - exercise 131, p144
  if (len > branchScore) {
    branchScore = len;
    bestBranchTail = k;
  }
  CHECK(0 <= f && f <= 3) << "f must be between 0 and 3, got f=" << f;
  switch (f) {
  case 0: // neither is unit
    if (head != tail) {
      k = head;
      goto D3;
    }
    break;
  case 1: // 2head is unit
  case 2: // 2head+1 is unit
    LOG << "D3: variable " << ToString(Var(head))
        << " has unit clauses: f=" << f;
    m[d + 1] = f + 3;
    tail = k;
    goto D5;
  case 3: // both are unit
    LOG << "D3: variable " << ToString(Var(head))
        << " has both types of unit clauses";
    goto D7;
  }

D4: // Two-way branch.
  tail = bestBranchTail;
  head = NEXT[tail];

  m[d + 1] = (W[2 * head] == 0) || (W[2 * head + 1] != 0);
  LOG << "D4: two-way branch with " << ToString(Var(head));

D5: // Move on.
  CHECK(1 <= head && head <= NumVars())
      << "head must be a valid variable, got head=" << head;
  ++d;
  h[d] = k = head;
  if (tail == k) {
    tail = 0;
  } else {
    NEXT[tail] = head = NEXT[k];
  }
  LOG << "D5: choose " << ToString(Lit(2 * k + (m[d] & 1))) << " and delete "
      << ToString(Var(k)) << " from the ring";

D6: // Update watches.
  x[k] = (m[d] + 1) & 1;
  for (int j = W[2 * k + x[k]]; j != 0;) {
    CHECK(L[START[j]] == (2 * k + x[k]))
        << "clause " << j << " should be watching "
        << ToString(Lit(2 * k + x[k])) << " but it's watching "
        << ToString(Lit(L[START[j]]));
    int ii = 0;
    for (int i = START[j] + 1; i < START[j - 1]; ++i) {
      // if L[i] is not set or already set to true, we can watch it.
      int xi = L[i] >> 1;
      if (x[xi] == -1 || x[xi] == (1 ^ (L[i] & 1))) {
        ii = i;
        break;
      }
    }
    CHECK(ii > 0) << "clause " << j << " should have an alternative watchee";
    // Update the watchee for clause j.
    int ll = L[ii]; // this is the new watched literal.
    int jj = j;
    LOG << "D6: clause " << j << " is now watching " << ToString(Lit(ll));

    // If the watch lists for the newly watched variable become non-empty,
    // we add the variable to the ring first.
    if (x[ll >> 1] == -1 && W[ll] == 0 && W[ll ^ 1] == 0) {
      int y = ll >> 1;
      LOG << "D6: variable " << ToString(Var(y)) << " added to ring";
      if (tail == 0) {
        head = tail = y;
      } else {
        NEXT[y] = head;
        head = y;
      }
      NEXT[tail] = head;
    }
    j = LINK[j];      // move forward to other clauses that watch ~l currently.
    LINK[jj] = W[ll]; // the updated clause is now the first one watching l'.
    W[ll] = jj;       // ...so we set it as the head of the list.
    std::swap(L[START[jj]], L[ii]); // move the watched literal to the head.
  }
  W[2 * k + x[k]] = 0;
  goto D2;

D7: // Backtrack.
  tail = k;
  while (m[d] >= 2) {
    k = h[d];
    CHECK(1 <= k && k <= NumVars())
        << "k must be a valid variable, got k=" << k;
    CHECK(x[k] != -1) << "x[" << k << "] must be set";
    x[k] = -1;
    // k is now active again, and if watched, it must be added back to the ring.
    if ((W[2 * k] != 0) || (W[2 * k + 1] != 0)) {
      NEXT[k] = head;
      head = k;
      NEXT[tail] = head;
      LOG << "D7: variable " << ToString(Var(k)) << " added to ring";
    }
    --d;
  }

D8: // Failure?
  if (d > 0) {
    m[d] = 3 - m[d];
    k = h[d];
    CHECK(x[k] != -1) << "switched variable k=" << k << " must still be set";
    LOG << "D8: variable " << ToString(Var(k)) << " switched";
    goto D6;
  } else {
    return {Result::kUNSAT, {}};
  }
}
std::pair<Result, std::vector<Assignment>> D::SolveAll() {
  COMMENT << "this solver does not support listing all satisfying assignments";
  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
