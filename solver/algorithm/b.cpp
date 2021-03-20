#include "solver/algorithm/b.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "util/check.h"

namespace solver {
namespace algorithm {

/*
 * 7.2.2.2 - Algorithm B - p31
 */
std::pair<Result, std::vector<Lit>> B::Solve() {
  // L[i]     = i-th cell's literal.
  // W[l]     = first clause watching literal l or 0 if none.
  // START[j] = first cell of clause j
  // LINK[j]  = next clause watching the same literal as clause j, or 0 if none.
  std::vector<int> L{0};
  std::vector<int> W(2 * NumVars() + 2, 0);
  std::vector<int> START(NumClauses() + 1, 0);
  std::vector<int> LINK(NumClauses() + 1, 0);

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

  std::vector<int> m(NumVars() + 1, 0);

B1: // Initialize.
  int d = 1;
  int l;

B2: // Rejoice or choose.
  if (d > NumVars()) {
    std::vector<Lit> ret;
    for (int j = 1; j <= NumVars(); ++j) {
      Var x(j);
      ret.push_back((1 ^ (m[j] & 1)) ? x : ~x);
    }
    return {Result::kSAT, ret};
  }
  // Choose ~l if W[l] is empty or W[~l] is not empty.
  m[d] = (W[2 * d] == 0 || W[2 * d + 1] != 0);
  l = 2 * d + m[d];
  std::clog << "B2: choose l=" << ToString(Lit(l)) << " m=";
  for (int j = 1; j <= d; ++j) {
    std::clog << m[j];
  }
  std::clog << std::endl;

B3: // Remove ~l if possible.
  for (int j = W[l ^ 1]; j != 0;) {
    CHECK("Clause should be watching ~l", L[START[j]] == (l ^ 1));
    int k = 0;
    for (int i = START[j] + 1; i < START[j - 1]; ++i) {
      // if L[i] is unknown or already set to true, we can watch it.
      int x = L[i] >> 1;
      if (x > d || (m[x] & 1) == (L[i] & 1)) {
        k = i;
        break;
      }
    }
    // If we cannot stop watching ~l.
    if (k == 0) {
      goto B5;
    }
    // Update the watchee for clause j otherwise.
    int ll = L[k]; // this is the new watched literal.
    int jj = j;
    std::clog << "B3: clause " << j << " is now watching " << ToString(Lit(ll))
              << std::endl;
    j = LINK[j];      // move forward to the clauses that watch ~l currently.
    LINK[jj] = W[ll]; // the updated clause is now the first one watching l'.
    W[ll] = jj;       // ...so we set it as the head of the list.
    std::swap(L[START[jj]], L[k]); // move the watched literal to the head.
    W[l ^ 1] = j; // ...and update the head of the list that watches ~l.
  }

B4: // Advance.
  W[l ^ 1] = 0;
  std::clog << "B4: literal " << ToString(Lit(l ^ 1))
            << " is not watched by any clause" << std::endl;
  ++d;
  goto B2;

B5: // Try again.
  if (m[d] < 2) {
    m[d] = 3 - m[d];
    l = 2 * d + (m[d] & 1);
    std::clog << "B5: try again with l=" << ToString(Lit(l)) << std::endl;
    goto B3;
  }

B6: // Backtrack.
  if (d == 1) {
    return {Result::kUNSAT, {}};
  }
  --d;
  std::clog << "B6: backtrack with d=" << d << std::endl;
  goto B5;
}

} // namespace algorithm
} // namespace solver
