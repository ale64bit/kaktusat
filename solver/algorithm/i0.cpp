#include "solver/algorithm/i0.h"

#include <algorithm>
#include <iomanip>
#include <queue>

#include "util/log.h"

namespace solver {
namespace algorithm {

std::pair<Result, std::vector<Lit>> I0::Solve() {
  int m; // current number of clauses.
  int d; // depth of implicit search tree.
  int i; // index of clause falsified by l.
  int j; // index of clause falsified by ~l.
  int t; // new depth to backtrack to on resolution.

  // Watch lists:
  //   W[l]    = first clause watching literal l or -1 if none.
  //   LINK[k] = next clause watching same literal as clause k or -1 if none.
  std::vector<int> W(2 * NumVars() + 2, -1);
  std::vector<int> LINK;

  // q    = queue of unset literals to make decisions.
  // l[d] = selected literal at level d.
  // dec[x] = dec at which the variable x was decided.
  std::queue<int> q;
  std::vector<int> l(NumVars() + 1, 0);
  std::vector<int> x(NumVars() + 1, -1);
  std::vector<int> dec(NumVars() + 1, -1);

  std::vector<Lit> Cm; // learned clause.

  m = NumClauses();

  // Build watch lists.
  for (int k = 0; k < m; ++k) {
    const int lk = clauses_[k][0].ID();
    LINK.push_back(W[lk]);
    W[lk] = k;
  }

  // Initially all literals all available for decision.
  for (int k = 1; k <= NumVars(); ++k) {
    Var x(k);
    q.push((~x).ID());
  }

  // Finds a clause falsified by setting the literal l and returns its index.
  // Returns -1 if no clause is satisfied.
  auto FindFalsified = [&](int l) -> int {
    for (int k = W[l ^ 1]; k != -1; k = LINK[k]) {
      CHECK(clauses_[k][0].ID() == (l ^ 1))
          << "clause " << k << " should be watching " << ToString(Lit(l ^ 1))
          << ", but it's watching " << ToString(clauses_[k][0]);
      bool falsified = true;
      // Try to watch another literal.
      for (int ii = 1; ii < (int)clauses_[k].size(); ++ii) {
        const int ll = clauses_[k][ii].ID();
        // If ll is not set, or it's set to a value that makes this clause true.
        if (x[ll >> 1] == -1 || x[ll >> 1] == (ll & 1)) {
          falsified = false;
          break;
        }
      }
      if (falsified) {
        return k;
      }
    }
    return -1;
  };

  // Updates the watchlists for clauses currently watching ~l.
  auto UpdateWatches =
      [&](int l) {
        for (int k = W[l ^ 1]; k != -1;) {
          CHECK(clauses_[k][0].ID() == (l ^ 1))
              << "clause " << k << " should be watching "
              << ToString(Lit(l ^ 1)) << ", but it's watching "
              << ToString(clauses_[k][0]);
          bool found = false;
          // Try to watch another literal.
          for (int ii = 1; ii < (int)clauses_[k].size(); ++ii) {
            const int ll = clauses_[k][ii].ID();
            // If ll is not set, or it's set to a value that makes this clause
            // true.
            if (x[ll >> 1] == -1 || x[ll >> 1] == (ll & 1)) {
              int kk = k;
              k = LINK[k]; // advance k, since this one won't be falsified.
              W[l ^ 1] = LINK[kk]; // clause k no longer watches l.
              LINK[kk] = W[ll];    // clause k now watches ll.
              W[ll] = kk;          // clause k is now the first watching ll.
              std::swap(clauses_[kk][0], clauses_[kk][ii]);
              found = true;
              break;
            }
          }
          CHECK(found) << "clause " << k << " must have an alternative watchee";
        }
        CHECK(W[l ^ 1] == -1)
            << "no clause can be watching ~l after updating watches";
      };

I1: // Initialize.
  LOG << "I1: initialize";
  d = 0;

I2: // Advance.
  if (d == NumVars()) {
    std::vector<Lit> sol;
    for (int k = 1; k <= d; ++k) {
      sol.push_back(Lit(l[k]));
    }
    return {Result::kSAT, sol};
  }
  CHECK(!q.empty()) << "there must be unset literals available";
  ++d;
  l[d] = q.front();
  q.pop();
  x[l[d] >> 1] = l[d] & 1;
  dec[l[d] >> 1] = d;
  LOG << "I2: l[" << d << "] = " << ToString(Lit(l[d]));

I3: // Find falsified C(i).
  i = FindFalsified(l[d]);
  if (i == -1) {
    LOG << "I3: no clause is falsified";
    UpdateWatches(l[d]);
    goto I2;
  }
  LOG << "I3: C(" << i << ") is falsified: " << ToString(clauses_[i]);

I4: // Find falsified C(j).
  l[d] ^= 1;
  x[l[d] >> 1] = l[d] & 1;
  LOG << "I4: l[" << d << "] = " << ToString(Lit(l[d]));
  j = FindFalsified(l[d]);
  if (j == -1) {
    LOG << "I4: no clause is falsified";
    UpdateWatches(l[d]);
    goto I2;
  }
  LOG << "I4: C(" << j << ") is falsified: " << ToString(clauses_[j]);

I5: // Resolve.
  CHECK(0 <= i && i < m) << "falsified clause must be valid, got i=" << i;
  CHECK(0 <= j && j < m) << "falsified clause must be valid, got j=" << j;
  CHECK(clauses_[i][0].ID() == l[d])
      << "falsified clause i=" << i << " should be watching "
      << ToString(Lit(l[d])) << " but it's watching "
      << ToString(clauses_[i][0]);
  CHECK(clauses_[j][0].ID() == (l[d] ^ 1))
      << "falsified clause j=" << j << " should be watching "
      << ToString(Lit(l[d] ^ 1)) << " but it's watching "
      << ToString(clauses_[j][0]);

  ++m;
  // Resolve C(m) = C(i) ⬦ C(j).
  Cm.clear();
  Cm.insert(Cm.end(), clauses_[i].begin() + 1, clauses_[i].end());
  Cm.insert(Cm.end(), clauses_[j].begin() + 1, clauses_[j].end());
  std::sort(Cm.begin(), Cm.end());
  Cm.erase(std::unique(Cm.begin(), Cm.end()), Cm.end());
  if (Cm.empty()) {
    return {Result::kUNSAT, {}};
  }
  // Find the decision level to backtrack next.
  t = -1;
  for (const auto &lit : Cm) {
    const int l = lit.ID();
    CHECK(x[l >> 1] != -1)
        << "all literals on resolved clauses must be decided";
    CHECK(x[l >> 1] != (l & 1))
        << "all literals on resolved clauses must falsify the resolved clause";
    t = std::max(t, dec[l >> 1]);
  }
  CHECK(t != -1) << "new depth after resolution must be valid";
  LOG << "I5: learned C(" << m - 1 << ")=(" << ToString(Cm) << ") from C(" << i
      << ")=(" << ToString(clauses_[i]) << ") ⬦ C(" << j << ")=("
      << ToString(clauses_[j]) << ")";
  for (int dd = t + 1; dd <= d; ++dd) {
    LOG << "I5: unset " << ToString(Lit(l[dd]).V());
    q.push(l[dd]);
    x[l[dd] >> 1] = -1;
    dec[l[dd] >> 1] = -1;
  }
  // Find the correct literal to watch, which will be negated in I4 next.
  for (int k = 1; k < (int)Cm.size(); ++k) {
    if (Cm[k].ID() == (l[t] ^ 1)) {
      std::swap(Cm[0], Cm[k]);
      break;
    }
  }
  // Update the watchlist for the learned clause.
  LINK.push_back(W[Cm[0].ID()]);
  W[Cm[0].ID()] = m - 1;
  clauses_.push_back(Cm);
  d = t;
  i = m - 1;
  goto I4;
}

} // namespace algorithm
} // namespace solver
