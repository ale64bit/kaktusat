#include "solver/algorithm/c.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <numeric>

#include "util/log.h"

namespace solver {
namespace algorithm {

// Watch lists for watching 2 literals per clause.
class WatchList {
public:
  // Iterator for iterating the watch list of a specific literal.
  class Iterator {
  public:
    Iterator(const std::vector<Clause> &clauses, int l, std::vector<int> &w,
             std::vector<std::array<int, 2>> &link)
        : clauses(clauses), l(l), w(w), link(link) {
      cur = w[l];
      ptr = &w[l];
    }

    // Index of the current clause watching l.
    const int operator*() const { return cur; }

    // Checks whether there are more clauses watching l.
    operator bool() const { return cur != -1; }

    // Advance to next clause watching l.
    Iterator &operator++() {
      CHECK(0 <= cur && cur < static_cast<int>(link.size()))
          << "current clause is out of range: cur=" << cur;
      ptr = &link[cur][clauses[cur][0] != l];
      cur = *ptr;
      return *this;
    }

    // Swaps the first and second watched literal for current clause.
    void Swap() {
      CHECK(0 <= cur && cur < static_cast<int>(link.size()))
          << "current clause is out of range: cur=" << cur;
      std::swap(link[cur][0], link[cur][1]);
    }

    // Move current clause's k-th from l's to ll's watch list and advances the
    // iterator.
    Iterator &MoveAndAdvance(int k) {
      CHECK(0 <= cur && cur < static_cast<int>(link.size()))
          << "current clause is out of range: cur=" << cur;
      CHECK(0 <= k && k < static_cast<int>(clauses[cur].size()))
          << "literal is out of range: k=" << k
          << " size=" << clauses[cur].size();
      const int ll = clauses[cur][k].ID();
      *ptr = link[cur][k];
      link[cur][k] = w[ll];
      w[ll] = cur;
      cur = *ptr;
      return *this;
    }

  private:
    const std::vector<Clause> &clauses;
    const int l;
    std::vector<int> &w;
    std::vector<std::array<int, 2>> &link;
    int cur;
    int *ptr;
  };

  WatchList(const std::vector<Clause> &clauses, int n)
      : clauses(clauses), w(2 * n + 2, -1) {}

  // Watches a new clause. Unit clauses are ignored but taken into account for
  // their indices.
  void Watch(const Clause &c) {
    link.push_back({-1, -1});
    const int idx = static_cast<int>(link.size() - 1);
    if (c.size() == 1) {
      return;
    }
    for (size_t off = 0; off < 2; ++off) {
      link[idx][off] = w[c[off].ID()];
      w[c[off].ID()] = idx;
    }
  }

  // Creates an Iterator for the watch list of literal l.
  Iterator Iterate(int l) { return Iterator(clauses, l, w, link); }

private:
  const std::vector<Clause> &clauses;
  std::vector<int> w;
  std::vector<std::array<int, 2>> link;
};

// A heap for managing activity scores to make branching decisions.
// @see: 7.2.2.2 - p67
template <typename T> class ActivityHeap {
  static constexpr double kDampingFactor = 0.95;
  static constexpr double kMaxActivity = 1e100;

public:
  ActivityHeap(int size)
      : n(size), h(size + 1), loc(size + 1), act(size + 1, 0),
        scalingFactor(1) {
    std::iota(h.begin(), h.end(), 0);
    std::iota(loc.begin(), loc.end(), 0);
  }

  // Removes and returns the variable with highest activity.
  int Pop() {
    int ret = h[1];
    loc[h[n]] = 1;
    h[1] = h[n];
    --n;
    Heapify(1);
    loc[ret] = 0;
    return ret;
  }

  // Adds a variable back to the heap.
  void Push(int k) {
    ++n;
    loc[k] = n;
    h[n] = k;
    FloatUp(n);
  }

  // Increments the activity of variable k by the scaling factor.
  // If overflow occurrs, scores and scaling factor are downscaled.
  void Inc(int k) {
    act[k] += scalingFactor;
    if (act[k] > kMaxActivity) {
      for (int i = 1; i <= n; ++i) {
        act[k] /= kMaxActivity;
      }
      scalingFactor /= kMaxActivity;
    }
    FloatUp(loc[k]);
  }

  // Applies the damping factor to the scaling factor after scores are updated.
  void Damp() { scalingFactor /= kDampingFactor; }

  // Checks whether variable k is in the heap.
  bool Contains(int k) const { return loc[k] > 0; }

  // Returns the number of variables currently in the heap.
  int Size() const { return n; }

  // Checks whether the heap is empty.
  bool Empty() const { return n == 0; }

private:
  int n;
  std::vector<int> h;
  std::vector<int> loc;
  std::vector<T> act;
  T scalingFactor;

  void FloatUp(int i) {
    for (; i > 1 && act[h[i >> 1]] < act[h[i]]; i >>= 1) {
      std::swap(loc[h[i >> 1]], loc[h[i]]);
      std::swap(h[i >> 1], h[i]);
    }
  }

  void Heapify(int i) {
    while (true) {
      int j = i;
      for (auto ii : {2 * i, 2 * i + 1}) {
        if (ii <= n && act[h[ii]] > act[h[j]]) {
          j = ii;
        }
      }
      if (j == i) {
        return;
      }
      std::swap(loc[h[i]], loc[h[j]]);
      std::swap(h[i], h[j]);
      i = j;
    }
  }
};

std::pair<Result, Assignment> C::Solve() {
  constexpr size_t kPurgeThreshold = std::numeric_limits<size_t>::max();
  constexpr size_t kFlushThreshold = std::numeric_limits<size_t>::max();

  // Trail data:
  //
  //   L      = trail literals
  //   reason = index of the reason clause for l or -1 if decision.
  //   lloc   = location of decision level d in the trail.
  //   b      = literals of learned clause when resolving conflicts.
  std::vector<int> L;
  std::vector<int> reason(2 * NumVars() + 2, -1);
  std::vector<int> lloc(NumVars() + 1, 0);
  std::vector<Lit> b;

  // Variable data:
  //
  //   stamp[k] = stamp number used when resolving conflicts.
  //   level[k] = current level k belongs to.
  //   val[k]   = current value of k (0=pos, 1=neg, -1=unset).
  //   old[k]   = old value of k, used for phase saving.
  //   tloc[k]  = location of k in the trail, of -1 if not in the trail.
  std::vector<int> stamp(NumVars() + 1, 0);
  std::vector<int> level(NumVars() + 1);
  std::vector<int> val(NumVars() + 1, -1);
  std::vector<int> old(NumVars() + 1, -1);
  std::vector<int> tloc(NumVars() + 1, -1);

  // Watch lists:
  //
  WatchList w(clauses_, NumVars());

  // Activity heap management:
  //
  ActivityHeap<double> heap(NumVars());

  // State:
  //
  int m;    // number of new clauses discovered.
  int d;    // decision level.
  int dd;   // backjump level after resolving a conflict.
  int l;    // currently selected literal.
  size_t g; // current trail position, behind the trail head.
  int cc;   // index of conflict clause.

  // Some closures to check whether a literal is currently free, true or false.
  auto IsFree = [&](const Lit &l) { return val[l.V().ID()] == -1; };
  auto IsTrue = [&](const Lit &l) {
    return !IsFree(l) && l.IsPos() == (val[l.V().ID()] == 0);
  };
  auto IsFalse = [&](const Lit &l) {
    return !IsFree(l) && l.IsPos() != (val[l.V().ID()] == 0);
  };

  // Builds a string representing the current trail. Useful for debugging and
  // reporting.
  auto TrailString = [&]() {
    std::stringstream out;
    out << "\n\t" << std::left << std::setw(4) << "t"
        << " " << std::setw(5) << "Lt"
        << " " << std::setw(5) << "level"
        << " reason" << '\n';
    for (size_t t = 0; t < L.size(); ++t) {
      out << "\t" << std::setw(4) << t << " " << std::setw(5 + (L[t] & 1))
          << ToString(Lit(L[t])) << " " << std::setw(5) << level[L[t] >> 1]
          << " "
          << (reason[L[t]] == -1 ? "Λ"
                                 : "(" + ToString(clauses_[reason[L[t]]]) + ")")
          << '\n';
    }
    return out.str();
  };

C1: // Initialize.
  L.reserve(NumVars() + 1);
  for (int i = 0; i < NumClauses(); ++i) {
    if (clauses_[i].empty()) { // empty clause
      return {Result::kUNSAT, {}};
    } else if (clauses_[i].size() == 1) { // unit clause
      LOG << "C1: L(" << L.size() << ")=" << ToString(clauses_[i][0])
          << " with reason (" << ToString(clauses_[i]) << ")";
      if (!IsFree(clauses_[i][0])) {
        COMMENT << "C1: instance contains contradictory clauses ("
                << ToString(clauses_[i][0]) << ") and ("
                << ToString(~clauses_[i][0]) << ")";
        return {Result::kUNSAT, {}};
      }
      const int x0 = clauses_[i][0].V().ID();
      tloc[x0] = static_cast<int>(L.size());
      val[x0] = clauses_[i][0].IsPos() ? 0 : 1;
      level[x0] = 0;
      L.push_back(clauses_[i][0].ID());
      reason[clauses_[i][0].ID()] = i;
    }
    w.Watch(clauses_[i]);
  }
  m = 0;
  d = 0;
  g = 0;

C2: // Level complete?
  if (g == L.size()) {
    LOG << "C2: level complete: G=F=" << g;
    goto C5;
  }

C3: // Advance G.
  CHECK(g < L.size()) << "G should be behind the trail: G=" << g
                      << " F=" << L.size();
  l = L[g];
  ++g;
  LOG << "C3: advance to G=" << g << ": l=" << ToString(Lit(l));

  for (auto it = w.Iterate(l ^ 1); it;) {
    auto &c = clauses_[*it];
    CHECK(c.size() > 1) << "unit clauses cannot appear in watch lists";
    CHECK(c[0].ID() == (l ^ 1) || c[1].ID() == (l ^ 1))
        << "clause #" << *it << " (" << ToString(c) << ") should be watching "
        << ToString(Lit(l ^ 1)) << ", but it's watching " << ToString(c[0])
        << " and " << ToString(c[1]);

  C4: // Does c force a unit?
    if (c[0].ID() == (l ^ 1)) {
      std::swap(c[0], c[1]);
      it.Swap();
    }
    if (IsTrue(c[0])) {
      // While c[0] is true, this clause won't be falsified.
      ++it;
      continue;
    }
    // Find an alternative literal to watch.
    bool hasAlt = false;
    for (size_t j = 2; j < c.size(); ++j) {
      if (!IsFalse(c[j])) {
        std::swap(c[1], c[j]);
        it.MoveAndAdvance(1);
        hasAlt = true;
        break;
      }
    }
    if (!hasAlt) {
      // If there is no alternative and c[0] is false, this is a conflict.
      if (IsFalse(c[0])) {
        cc = *it;
        goto C7;
      } else {
        // Otherwise, we can set c[0] if it's free.
        CHECK(IsFree(c[0]))
            << "literal " << ToString(c[0]) << " must be free at this point.";
        LOG << "C4: L(" << L.size() << ")=" << ToString(c[0])
            << " forced with reason (" << ToString(clauses_[*it]) << ")";
        const int l0 = c[0].ID();
        const int x0 = c[0].V().ID();
        tloc[x0] = static_cast<int>(L.size());
        val[x0] = c[0].IsPos() ? 0 : 1;
        level[x0] = d;
        L.push_back(l0);
        reason[l0] = *it;
        ++it;
      }
    }
  }
  goto C2; // no conflicts.

C5: // New level?
  if (L.size() == NumVars()) {
    Assignment sol;
    for (const auto &l : L) {
      sol.emplace_back(l);
    }
    return {Result::kSAT, sol};
  } else if (m >= kPurgeThreshold) {
    CHECK(false) << "purge not implemented";
    // TODO purge
  } else if (m >= kFlushThreshold) {
    CHECK(false) << "flush not implemented";
    // TODO flush
    goto C2;
  } else {
    ++d;
    lloc[d] = static_cast<int>(L.size());
    LOG << "C5: new level d=" << d << " created at i[" << d << "]=" << lloc[d];
  }

C6: // Make a decision.
  while (true) {
    CHECK(!heap.Empty()) << "there must be at least a variable to decide"
                         << TrailString();
    auto k = heap.Pop();
    if (val[k] >= 0) {
      continue;
    }

    l = 2 * k + (old[k] & 1);
    val[k] = (old[k] & 1);
    level[k] = d;
    tloc[k] = static_cast<int>(L.size());
    L.push_back(l);
    reason[l] = -1;
    LOG << "C6: L[" << L.size() - 1 << "]=" << ToString(Lit(l))
        << " by decision";
    CHECK(L.size() == g + 1)
        << "the trace should be only one step ahead: G=" << g
        << " F=" << L.size();
    goto C3;
  }

C7: // Resolve a conflict.
  if (d == 0) {
    return {Result::kUNSAT, {}};
  } else {
    LOG << "C7: resolving conflict clause (" << ToString(clauses_[cc]) << ")";
    CHECK(clauses_[cc][1].ID() == (l ^ 1))
        << "conflict clause cc=" << cc << " should be watching "
        << ToString(Lit(l ^ 1));
    for (const auto &ll : clauses_[cc]) {
      CHECK(IsFalse(ll)) << "a conflict clause must be falsified";
    }

    int dcnt = 0;
    b.clear();
    for (const auto &ll : clauses_[cc]) {
      const int li = (~ll).ID();
      const int ai = li >> 1;
      if (stamp[ai] < m + 1) {
        stamp[ai] = m + 1;
        heap.Inc(ai);
        if (level[ai] == d) {
          ++dcnt;
        } else {
          b.push_back(ll);
        }
      }
    }
    for (size_t tt = L.size() - 1;; --tt) {
      // We only consider stamped literals.
      if (stamp[L[tt] >> 1] == m + 1) {
        // When there's only one literal left from level d, we complete the
        // learned claused.
        if (dcnt == 1) {
          b.push_back(~Lit(L[tt]));
          break;
        }
        CHECK(reason[L[tt]] != -1)
            << "reasons during clause learning cannot be decisions: t'=" << tt
            << " L[" << tt << "]=" << L[tt];
        for (const auto &ll : clauses_[reason[L[tt]]]) {
          const int li = (~ll).ID();
          const int ai = li >> 1;
          if (stamp[ai] < m + 1) {
            stamp[ai] = m + 1;
            heap.Inc(ai);
            if (level[ai] == d) {
              ++dcnt;
            } else {
              b.push_back(ll);
            }
          }
        }
        --dcnt;
      }
    }
    CHECK(level[b.back().V().ID()] == d)
        << "l'=" << ToString(b.back()) << " should be on level d=" << d
        << " but it is on level " << level[b.back().V().ID()];
    for (size_t i = 0; i < b.size() - 1; ++i) {
      CHECK(level[b[i].V().ID()] < d)
          << "there must be a single literal in level d in clause ("
          << ToString(b) << ")" << TrailString();
    }
    // Calculate the backjump level.
    dd = 0;
    for (const auto &ll : b) {
      if (level[ll.V().ID()] < d) {
        dd = std::max(dd, level[ll.V().ID()]);
      }
    }
    CHECK(dd < d) << "backjump level d'=" << dd
                  << " should be below current level d=" << d;
  }

C8: // Backjump.
  while (L.size() > lloc[dd + 1]) {
    l = L.back();
    const int k = l >> 1;
    old[k] = val[k];
    val[k] = -1;
    reason[l] = -1;
    if (!heap.Contains(k)) {
      heap.Push(k);
    }
    L.pop_back();
  }
  g = L.size();
  d = dd;
  LOG << "C8: backjump to d=" << d << " and G=" << g;

C9: // Learn.
  const int ll = b.back().ID();
  const int k = ll >> 1;
  // Add new clause.
  clauses_.push_back(b);
  ++m;
  // Update variable data.
  val[k] = ll & 1;
  level[k] = d;
  tloc[k] = static_cast<int>(L.size());
  // Update trail.
  L.push_back(ll);
  reason[ll] = static_cast<int>(clauses_.size() - 1);
  // Update activity.
  heap.Damp();
  // Update watches.
  w.Watch(clauses_.back());

  LOG << "C9: learned clause (" << ToString(b) << ")";
  goto C3;
}

std::pair<Result, std::vector<Assignment>> C::SolveAll() {
  COMMENT << "this solver does not support listing all satisfying assignments";
  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
