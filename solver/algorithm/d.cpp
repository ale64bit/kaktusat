#include "solver/algorithm/d.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "util/check.h"

namespace solver {
namespace algorithm {

std::pair<Result, std::vector<Lit>> D::Solve() {
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

  // m[j] = 0: trying x[h[j]], didn't try ~x[h[j]] yet.
  // m[j] = 1: trying ~x[h[j]], didn't try x[h[j]] yet.
  // m[j] = 2: trying x[h[j]], after ~x[h[j]] failed.
  // m[j] = 3: trying ~x[h[j]], after x[h[j]] failed.
  // m[j] = 4: trying x[h[j]], forced by unit clause.
  // m[j] = 5: trying ~x[h[j]], forced by unit clause.
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

  int d = 0; // depth of implicit search tree.
  int k;     // current variable.
  int f;     // whether a literal is unit.
  m[0] = 0;
  head = 0;
  tail = 0;
  for (int i = NumVars(); i >= 1; --i) {
    x[i] = -1;
    // If a variable is watched, add it to the ring.
    if ((W[2 * i] != 0) || (W[2 * i + 1] != 0)) {
      std::clog << "D1: variable " << ToString(Var(i)) << " added to ring"
                << std::endl;
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
    for (int j = 1; j <= NumVars(); ++j) {
      Var y(j);
      ret.push_back(x[j] ? y : ~y);
    }
    return {Result::kSAT, ret};
  }
  k = tail;

D3: // Look for unit clauses.
  head = NEXT[k];
  CHECK("head must be a valid variable", 1 <= head && head <= NumVars());
  // Check if variable k is watched by some unit clause.
  f = 0;
  for (int l : {2 * head + 1, 2 * head}) {
    f *= 2;
    for (int j = W[l]; j != 0; j = LINK[j]) {
      CHECK("Clause j must but currently watching l", L[START[j]] == l);
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
    }
  }
  CHECK("f must be between 0 and 3", 0 <= f && f <= 3);
  switch (f) {
  case 0: // neither is unit
    if (head != tail) {
      k = head;
      goto D3;
    }
    break;
  case 1: // 2head is unit
  case 2: // 2head+1 is unit
    std::clog << "D3: variable " << ToString(Var(head))
              << " has unit clauses: f=" << f << std::endl;
    m[d + 1] = f + 3;
    tail = k;
    goto D5;
  case 3: // both are unit
    std::clog << "D3: variable " << ToString(Var(head))
              << " has both types of unit clauses" << std::endl;
    goto D7;
  }

D4: // Two-way branch.
  head = NEXT[tail];
  m[d + 1] = (W[2 * head] == 0) || (W[2 * head + 1] != 0);
  std::clog << "D4: two-way branch with " << ToString(Var(head)) << std::endl;

D5: // Move on.
  CHECK("head must be a valid variable", 1 <= head && head <= NumVars());
  ++d;
  h[d] = k = head;
  if (tail == k) {
    tail = 0;
  } else {
    NEXT[tail] = head = NEXT[k];
  }
  std::clog << "D5: choose " << ToString(Lit(2 * k + (m[d] & 1)))
            << " and delete " << ToString(Var(k)) << " from the ring"
            << std::endl;

D6: // Update watches.
  x[k] = (m[d] + 1) & 1;
  for (int j = W[2 * k + x[k]]; j != 0;) {
    CHECK("Clause should be watching ~l", L[START[j]] == (2 * k + x[k]));
    int ii = 0;
    for (int i = START[j] + 1; i < START[j - 1]; ++i) {
      // if L[i] is not set or already set to true, we can watch it.
      int xi = L[i] >> 1;
      if (x[xi] == -1 || x[xi] == (1 ^ (L[i] & 1))) {
        ii = i;
        break;
      }
    }
    CHECK("Clause should have an alternative watchee", ii > 0);
    // Update the watchee for clause j.
    int ll = L[ii]; // this is the new watched literal.
    int jj = j;
    std::clog << "D6: clause " << j << " is now watching " << ToString(Lit(ll))
              << std::endl;
    // If the watch lists for the newly watched variable become non-empty,
    // we add the variable to the ring first.
    if (x[ll >> 1] == -1 && W[ll] == 0 && W[ll ^ 1] == 0) {
      int y = ll >> 1;
      std::clog << "D6: variable " << ToString(Var(y)) << " added to ring"
                << std::endl;
      NEXT[y] = head;
      head = y;
      NEXT[tail] = head;
    }
    j = LINK[j];      // move forward to other clauses that watch ~l currently.
    LINK[jj] = W[ll]; // the updated clause is now the first one watching l'.
    W[ll] = jj;       // ...so we set it as the h of the list.
    std::swap(L[START[jj]], L[ii]); // move the watched literal to the head.
  }
  W[2 * k + x[k]] = 0;
  goto D2;

D7: // Backtrack.
  tail = k;
  while (m[d] >= 2) {
    k = h[d];
    CHECK("k must be a valid variable", 1 <= k && k <= NumVars());
    x[k] = -1;
    // k is now active again, and if watched, it must be added back to the ring.
    if ((W[2 * k] != 0) || (W[2 * k + 1] != 0)) {
      NEXT[k] = head;
      head = k;
      NEXT[tail] = head;
      std::clog << "D7: variable " << ToString(Var(k)) << " added to ring"
                << std::endl;
    }
    --d;
  }

D8: // Failure?
  if (d > 0) {
    m[d] = 3 - m[d];
    k = h[d];
    std::clog << "D8: variable " << ToString(Var(k)) << " switched"
              << std::endl;
    goto D6;
  } else {
    return {Result::kUNSAT, {}};
  }
}

} // namespace algorithm
} // namespace solver
