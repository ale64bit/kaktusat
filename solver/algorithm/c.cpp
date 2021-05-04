#include "solver/algorithm/c.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <type_traits>

#include "util/log.h"

namespace solver {
namespace algorithm {

struct Stats {
  int64_t decisions = 0;
  int64_t conflicts = 0;
  int64_t propagations = 0;
  int64_t restarts = 0;
  int64_t learnedClauses = 0;
  int64_t clauseLength = 0;
  int64_t purged = 0;

  std::string ToString() const {
    std::stringstream out;
    out << "decisions=" << decisions << " conflicts=" << conflicts
        << " propagations=" << propagations << " restarts=" << restarts
        << " learned=" << learnedClauses << " purged=" << purged
        << std::setprecision(1) << std::fixed << " avgClauseLen="
        << static_cast<double>(clauseLength) / learnedClauses;
    return out.str();
  }
};

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

    // Erase current clause from l's watch list and advances the iterator.
    Iterator &Erase() {
      CHECK(0 <= cur && cur < static_cast<int>(link.size()))
          << "current clause is out of range: cur=" << cur;
      *ptr = link[cur][clauses[cur][0] != l];
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

  // Rebuilds the watch lists from scratch using the underlying clauses.
  void Rebuild() {
    std::fill(w.begin(), w.end(), -1);
    link.resize(clauses.size(), {-1, -1});
    for (size_t i = 0; i < clauses.size(); ++i) {
      Watch(i);
    }
  }

  // Watches clause at index k. Unit clauses are ignored.
  void Watch(int k) {
    link.resize(clauses.size(), {-1, -1});
    const Clause &c = clauses[k];
    if (c.size() == 1) {
      return;
    }
    for (size_t off = 0; off < 2; ++off) {
      link[k][off] = w[c[off].ID()];
      w[c[off].ID()] = k;
    }
  }

  // Removes the clause with index k from the watch lists.
  void Forget(int k) {
    for (int off = 0; off < 2; ++off) {
      auto it = Iterate(clauses[k][off].ID());
      while (*it != k) {
        ++it;
      }
      CHECK(*it == k) << "forgotten clause must appear in the watch list";
      it.Erase();
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
  static constexpr T kDampingFactor = 0.95;
  static constexpr T kMaxActivity = 1e100;

public:
  ActivityHeap(int size, std::mt19937 &rng)
      : n(size), size(size), h(size + 1), loc(size + 1), act(size + 1, 0),
        scalingFactor(1) {

    std::vector<int> p(n);
    std::iota(p.begin(), p.end(), 1);
    std::shuffle(p.begin(), p.end(), rng);

    std::copy(p.begin(), p.end(), h.begin() + 1);
    for (int k = 1; k <= n; ++k) {
      loc[h[k]] = k;
    }
  }

  // Removes and returns the variable with highest activity.
  int Pop() {
    CHECK(!Empty()) << "heap cannot be empty";
    int ret = h[1];
    loc[h[size]] = 1;
    h[1] = h[size];
    --size;
    Heapify(1);
    loc[ret] = 0;
    return ret;
  }

  int Top() const {
    CHECK(!Empty()) << "heap cannot be empty";
    return h[1];
  }

  // Adds a variable back to the heap.
  void Push(int k) {
    CHECK(loc[k] == 0) << "duplicate variable insert: k=" << k;
    ++size;
    loc[k] = size;
    h[size] = k;
    FloatUp(size);
  }

  // Increments the activity of variable k by the scaling factor.
  // If overflow occurrs, scores and scaling factor are downscaled.
  void Inc(int k) {
    CHECK(scalingFactor > 0)
        << "scaling factor must be positive to maintain the heap invariant";
    act[k] += scalingFactor;
    if (act[k] > kMaxActivity) {
      for (int i = 1; i <= n; ++i) {
        act[i] /= kMaxActivity;
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
  int Size() const { return size; }

  // Checks whether the heap is empty.
  bool Empty() const { return size == 0; }

  // Returns the activity of variable k.
  T GetActivity(int k) const { return act[k]; }

  // Utility function to verify the integrity of the heap in debug builds.
  void CheckIntegrity() const {
    CHECK(std::isfinite(scalingFactor))
        << "corrupt heap: scalingFactor=" << scalingFactor;
    for (int k = 1; k <= n; ++k) {
      CHECK(std::isfinite(act[k]))
          << "corrupt heap: act[" << k << "]=" << act[k];
      CHECK(act[k] <= kMaxActivity)
          << "corrupt heap: act[" << k << "]=" << act[k]
          << " > kMaxActivity=" << kMaxActivity;
      if (loc[k] > 0) {
        CHECK(h[loc[k]] == k)
            << "corrupt heap: misplaced variable k=" << k << ": loc[" << k
            << "]=" << loc[k] << " but h[" << loc[k] << "]=" << h[loc[k]];
      }
    }
    for (int j = 2; j <= size; ++j) {
      CHECK(act[h[j >> 1]] >= act[h[j]])
          << "corrupt heap: n=" << n << " h[" << (j >> 1) << "]=" << h[j >> 1]
          << " h[" << j << "]=" << h[j] << " act[" << h[j >> 1]
          << "]=" << act[h[j >> 1]] << " act[" << h[j] << "]=" << act[h[j]]
          << " loc[" << h[j >> 1] << "]=" << loc[h[j >> 1]] << " loc[" << h[j]
          << "]=" << loc[h[j]];
    }
  }

private:
  const int n;
  int size;
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
        if (ii <= size && act[h[ii]] > act[h[j]]) {
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

// Reluctant doubling sequence: 1,1,2,1,1,2,4,1,1,2,1,1,2,4,8,1,1,2,1,1,...
//
// @see: 7.2.2.2 - (130), p80
// @see: Luby, M., Sinclair, A., Zuckerman, D.: Optimal speedup of Las Vegas
//       algorithms. Information Processing Letters. 47, 173–180 (1993).
//       https://doi.org/10.1016/0020-0190(93)90029-9
template <typename T> class ReluctantDoublingGenerator {
  static_assert(std::is_signed<T>::value, "the generator type must be signed");

public:
  ReluctantDoublingGenerator() : u(1), v(1) {}

  T operator()() {
    T ret = v;
    if ((u & -u) == v) {
      ++u;
      v = 1;
    } else {
      v *= 2;
    }
    return ret;
  }

private:
  T u;
  T v;
};

std::pair<Result, Assignment> C::Solve() {
  // Flushing/restart parameters.
  constexpr float kPsi = 1.f / 6;
  constexpr float kTheta = 17.f / 16;

  const size_t kFirstLearnedClause = clauses_.size();

  std::random_device rdev;
  std::mt19937 rng(rdev());
  std::bernoulli_distribution randDecision(.02);

  // Trail data:
  //
  //   L      = trail literals
  //   reason = index of the reason clause for l or -1 if decision.
  //   lloc   = location of decision level d in the trail.
  //   b      = literals of learned clause when resolving conflicts.
  //   used   = whether a clause is the reason of a literal in the trail.
  std::vector<int> L;
  std::vector<int> reason(2 * NumVars() + 2, -1);
  std::vector<int> lloc(NumVars() + 1, 0);
  std::vector<Lit> b;
  std::vector<int> used;

  // Variable data:
  //
  //   latestStamp     = latest stamp number in use.
  //   stamp[k]        = stamp number used when resolving conflicts.
  //   level[k]        = current level k belongs to.
  //   val[k]          = current value of k (0=pos, 1=neg, -1=unset).
  //   old[k]          = old value of k, used for phase saving.
  //   tloc[k]         = location of k in the trail, of -1 if not in the trail.
  //   redundant[k]    = cache to check whether a literal is redundant with
  //                     respect to learned clause. Positive/negative stamp
  //                     values represent true/false values, respectively.
  //   learnedStamp[k] = stamp at which a literal was last learned. Useful for
  //                     detecting immediately subsumed learned clauses. (TODO:
  //                     can it be removed/replaced with stamp[k]?)
  int latestStamp = 1;
  std::vector<int> stamp(NumVars() + 1, 0);
  std::vector<int> level(NumVars() + 1);
  std::vector<int> val(NumVars() + 1, -1);
  std::vector<int> old(NumVars() + 1, 1);
  std::vector<int> tloc(NumVars() + 1, -1);
  std::vector<int> redundant(2 * NumVars() + 2, 0);
  std::vector<int> learnedStamp(2 * NumVars() + 2, -1);

  // Watch lists:
  //
  WatchList w(clauses_, NumVars());

  // Activity heap management:
  //
  ActivityHeap<double> heap(NumVars(), rng);
  heap.CheckIntegrity();

  // Flushing and restarts:
  //
  // @see: 7.2.2.2 - p75
  // @see: Biere, A.: Adaptive Restart Strategies for Conflict Driven SAT
  //       Solvers. In: Theory and Applications of Satisfiability Testing – SAT
  //       2008. pp. 28–33. Springer Berlin Heidelberg (2008)
  int flushThreshold = 100;
  uint32_t agility = 0;
  ReluctantDoublingGenerator<int> rdgen;

  // Purging:
  //
  bool purging = false;
  int purgeLevel = -1;
  int purgeThreshold = 10000;

  // Stats:
  //
  Stats stats;

  // State:
  //
  int m = 0;    // number of new clauses discovered.
  int d = 0;    // decision level.
  int dd;       // backjump level after resolving a conflict.
  int l;        // currently selected literal.
  size_t g = 0; // current trail position, behind the trail head.
  int cc;       // index of conflict clause.

  // Some closures to check whether a literal is currently free, true or false.
  auto IsFree = [&](const Lit &l) { return val[l.VID()] == -1; };
  auto IsTrue = [&](const Lit &l) {
    return !IsFree(l) && l.IsPos() == (val[l.VID()] == 0);
  };
  auto IsFalse = [&](const Lit &l) {
    return !IsFree(l) && l.IsPos() != (val[l.VID()] == 0);
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

  // Literal lit is redundant with respect to clause c and current trail if:
  //   (a) ~lit is in the trail and either
  //       (i) ~lit is defined at level 0 or
  //       (ii) ~lit is not a decision and every false literal in ~lit's reason
  //            is either in c or redundant itself.
  // @see: 7.2.2.2 - exercise 257, p155
  std::function<bool(const Clause &, const Lit &)> IsRedundant =
      [&](const Clause &c, const Lit &lit) -> bool {
    const auto li = lit.ID();
    const auto xi = lit.VID();
    if (std::abs(redundant[li]) == latestStamp) {
      return redundant[li] > 0;
    }
    if (level[xi] == 0) {
      redundant[li] = latestStamp;
      return true;
    }
    if (reason[li ^ 1] == -1) {
      redundant[li] = -latestStamp;
      return false;
    }
    for (const auto &ll : clauses_[reason[li ^ 1]]) {
      const auto xj = ll.VID();
      if (IsFalse(ll)) {
        // Instead of checking naively whether xj belongs to the learned clause,
        // we can use the stamp and level.
        if ((stamp[xj] == latestStamp && level[xj] < d) || IsRedundant(c, ll)) {
        } else {
          redundant[li] = -latestStamp;
          return false;
        }
      }
    }
    redundant[li] = latestStamp;
    return true;
  };

C1: // Initialize.
  L.reserve(NumVars() + 1);
  for (int i = 0; i < NumClauses(); ++i) {
    used.push_back(0);
    if (clauses_[i].empty()) { // empty clause
      return {Result::kUNSAT, {}};
    } else if (clauses_[i].size() == 1) { // unit clause
      LOG << "C1: L(" << L.size() << ")=" << ToString(clauses_[i][0])
          << " with reason (" << ToString(clauses_[i]) << ")";
      if (!IsFree(clauses_[i][0])) {
        COMMENT << "C1: instance contains contradictory unit clauses ("
                << ToString(clauses_[i][0]) << ") and ("
                << ToString(~clauses_[i][0]) << ")";
        return {Result::kUNSAT, {}};
      }
      const int x0 = clauses_[i][0].VID();
      tloc[x0] = static_cast<int>(L.size());
      val[x0] = clauses_[i][0].IsPos() ? 0 : 1;
      level[x0] = 0;
      L.push_back(clauses_[i][0].ID());
      reason[clauses_[i][0].ID()] = i;
      used.back() = clauses_[i][0].ID();
    }
    w.Watch(i);
  }

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
  LOG << "C3: advance to G=" << g - 1 << ": l=" << ToString(Lit(l));

  cc = -1;
  for (auto it = w.Iterate(l ^ 1); it;) {
    auto &c = clauses_[*it];
    CHECK(c.size() > 1) << "unit clauses cannot appear in watch lists";
    CHECK(c[0].ID() == (l ^ 1) || c[1].ID() == (l ^ 1))
        << "clause #" << *it << " (" << ToString(c) << ") should be watching "
        << ToString(Lit(l ^ 1)) << ", but it's watching " << ToString(c[0])
        << " and " << ToString(c[1]) << TrailString();

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
        // If we are doing a purging run, we can continue and ignore the
        // conflicts for now.
        if (purging) {
          ++it;
        } else {
          cc = *it;
          goto C7;
        }
      } else {
        // Otherwise, we can set c[0] if it's free.
        CHECK(IsFree(c[0]))
            << "literal " << ToString(c[0]) << " must be free at this point.";
        LOG << "C4: L(" << L.size() << ")=" << ToString(c[0])
            << " forced with reason (" << ToString(clauses_[*it]) << ")";
        const int l0 = c[0].ID();
        const int x0 = c[0].VID();
        tloc[x0] = static_cast<int>(L.size());
        val[x0] = c[0].IsPos() ? 0 : 1;
        level[x0] = d;
        L.push_back(l0);
        used[*it] = l0;
        reason[l0] = *it;
        agility = agility - (agility >> 13) + (((old[x0] - val[x0]) & 1) << 19);
        ++it;
        ++stats.propagations;
      }
    }
  }
  goto C2; // no conflicts.

C5: // New level?
  if (L.size() == NumVars()) {
    // Check if we are doing a purging run, in which case, the trail now
    // contains every variable and we can compute the literal block distance for
    // each clause. Otherwise, we found a satisfiable assignment.
    if (!purging) {
      Assignment sol;
      for (const auto &l : L) {
        sol.emplace_back(l);
      }
      return {Result::kSAT, sol};
    } else {
      constexpr float kAlpha = 15.f / 16;
      const int kKeepTarget = (clauses_.size() - kFirstLearnedClause) / 2;
      std::vector<int> range(clauses_.size() - kFirstLearnedClause, 0);
      std::array<int, 257> rangeFreq;
      std::fill(rangeFreq.begin(), rangeFreq.end(), 0);
      for (size_t i = kFirstLearnedClause; i < clauses_.size(); ++i) {
        const size_t idx = i - kFirstLearnedClause;
        if (!used[i]) {
          // Compute literal block distance.
          int p = 0;
          int q = 0;
          std::vector<std::array<int, 2>> levelFreq(NumVars() + 1, {0, 0});
          for (const auto &lit : clauses_[i]) {
            if (level[lit.VID()] == 0) {
              range[idx] = 256;
              break;
            }
            ++levelFreq[level[lit.VID()]][lit.IsNeg()];
          }
          if (range[idx] < 256) {
            for (int i = 0; i <= NumVars(); ++i) {
              if (levelFreq[i][0] > 0) {
                ++p;
              } else if (levelFreq[i][1] > 0) {
                ++q;
              }
            }
            range[idx] = std::min(
                static_cast<int>(std::floor(16 * (p + kAlpha * q))), 255);
          }
        }
        ++rangeFreq[range[idx]];
      }

      int targetRange = 1;
      int sum = rangeFreq[0];
      while (targetRange < 256 && sum + rangeFreq[targetRange] <= kKeepTarget) {
        sum += rangeFreq[targetRange];
        ++targetRange;
      }
      for (size_t i = kFirstLearnedClause; i < clauses_.size();) {
        if (range[i - kFirstLearnedClause] >= targetRange) {
          LOG << "C5: purged clause (" << ToString(clauses_[i]) << ")";
          std::swap(clauses_[i], clauses_.back());
          std::swap(range[i - kFirstLearnedClause], range.back());
          if (used.back()) {
            reason[used.back()] = i;
          }
          used[i] = used.back();
          clauses_.pop_back();
          range.pop_back();
          used.pop_back();
          ++stats.purged;
        } else {
          ++i;
        }
      }
      w.Rebuild();

      // Backjump to original decision level where purge was started.
      while (L.size() > lloc[purgeLevel + 1]) {
        l = L.back();
        const int k = l >> 1;
        old[k] = val[k];
        val[k] = -1;
        level[k] = -1;
        if (reason[l] != -1) {
          Clause &c = clauses_[reason[l]];
          const bool falsified = (IsFalse(c[0]) && level[c[0].VID()] <= dd) ||
                                 (IsFalse(c[1]) && level[c[1].VID()] <= dd);
          if (falsified) {
            w.Forget(reason[l]);
            for (size_t off = 0; off < 2; ++off) {
              if (IsFalse(c[off]) && level[c[off].VID()] <= dd) {
                for (size_t i = 1 + off; i < c.size(); ++i) {
                  if (!IsFalse(c[i]) || level[c[i].VID()] > dd) {
                    std::swap(c[off], c[i]);
                    break;
                  }
                }
              }
            }
            w.Watch(reason[l]);
          }

          used[reason[l]] = 0;
          reason[l] = -1;
        }
        if (!heap.Contains(k)) {
          heap.Push(k);
        }
        L.pop_back();
      }
      g = L.size();
      d = purgeLevel;
      // End purge run.
      purging = false;
      purgeLevel = 0;
      LOG << "C5: purge completed";
    }
  } else if (!purging && m >= purgeThreshold) {
    LOG << "C5: purge run begin at level d=" << d;
    purging = true;
    purgeLevel = d;
    purgeThreshold = m + 10000;
  } else if (!purging && m >= flushThreshold) {
    const int delta = rdgen();
    flushThreshold = m + delta;
    bool flush = false;
    const float a = agility / std::pow(2.f, 32);
    // Flush schedule according to 7.2.2.2 - Table 4, p76.
    for (int i = 0; i < 16; ++i) {
      if (delta == (1 << i)) {
        flush = a <= std::pow(kTheta, i) * kPsi;
        break;
      }
    }
    if (flush) {
      int xk = heap.Top();
      while (val[xk] >= 0) {
        heap.Pop();
        xk = heap.Top();
      }
      CHECK(xk > 0) << "invalid unassigned variable of maximum activity: xk="
                    << xk;
      dd = 0;
      while (dd < d &&
             heap.GetActivity(L[lloc[dd]] >> 1) >= heap.GetActivity(xk)) {
        ++dd;
      }
      if (dd < d) {
        LOG << "C5: flushing from d=" << d << " to d'=" << dd;
        ++stats.restarts;
        while (L.size() > lloc[dd + 1]) {
          l = L.back();
          const int k = l >> 1;
          old[k] = val[k];
          val[k] = -1;
          level[k] = -1;
          if (reason[l] != -1) {
            Clause &c = clauses_[reason[l]];
            const bool falsified = (IsFalse(c[0]) && level[c[0].VID()] <= dd) ||
                                   (IsFalse(c[1]) && level[c[1].VID()] <= dd);
            if (falsified) {
              w.Forget(reason[l]);
              for (size_t off = 0; off < 2; ++off) {
                if (IsFalse(c[off]) && level[c[off].VID()] <= dd) {
                  for (size_t i = 1 + off; i < c.size(); ++i) {
                    if (!IsFalse(c[i]) || level[c[i].VID()] > dd) {
                      std::swap(c[off], c[i]);
                      break;
                    }
                  }
                }
              }
              w.Watch(reason[l]);
            }

            used[reason[l]] = 0;
            reason[l] = -1;
          }
          if (!heap.Contains(k)) {
            heap.Push(k);
          }
          L.pop_back();
        }
        g = L.size();
        d = dd;
        goto C2;
      }
    }
  }

  ++d;
  lloc[d] = static_cast<int>(L.size());
  LOG << "C5: new level d=" << d << " started at i[" << d << "]=" << lloc[d];

C6 : // Make a decision.
{
  int k;
  // With small probability, take a random decision rather than using the
  // variable with maximum activity score.
  // @see: 7.2.2.2 - exercise 267, p155
  if (randDecision(rng)) {
    std::vector<int> free;
    for (int k = 1; k <= NumVars(); ++k) {
      if (val[k] == -1) {
        free.push_back(k);
      }
    }
    std::shuffle(free.begin(), free.end(), rng);

    k = free[0];
    l = 2 * k + (old[k] & 1);
    LOG << "C6: L[" << L.size() << "]=" << ToString(Lit(l))
        << " by random decision";
  } else {
    while (true) {
      heap.CheckIntegrity();
      CHECK(!heap.Empty()) << "there must be at least a variable to decide"
                           << TrailString();
      k = heap.Pop();
      if (val[k] >= 0) {
        continue;
      }
      l = 2 * k + (old[k] & 1);
      LOG << "C6: L[" << L.size() << "]=" << ToString(Lit(l)) << " by decision";
      break;
    }
  }

  ++stats.decisions;
  if (stats.decisions % 10000 == 0) {
    LOG << "C6: stats: agility=" << (agility / std::pow(2, 32)) << " "
        << stats.ToString();
  }

  val[k] = l & 1;
  level[k] = d;
  tloc[k] = static_cast<int>(L.size());
  reason[l] = -1;
  L.push_back(l);
  agility = agility - (agility >> 13) + (((old[k] - val[k]) & 1) << 19);
  CHECK(L.size() == g + 1) << "the trace should be only one step ahead: G=" << g
                           << " F=" << L.size();
  goto C3;
}

C7: // Resolve a conflict.
  if (d == 0) {
    COMMENT << "C6: stats: agility=" << (agility / std::pow(2, 32)) << " "
            << stats.ToString();
    return {Result::kUNSAT, {}};
  } else {
    LOG << "C7: resolving conflict clause (" << ToString(clauses_[cc]) << ")"
        << TrailString();
    CHECK(clauses_[cc][1].ID() == (l ^ 1))
        << "conflict clause cc=" << cc << " should be watching "
        << ToString(Lit(l ^ 1));
    for ([[maybe_unused]] const auto &ll : clauses_[cc]) {
      CHECK(IsFalse(ll)) << "a conflict clause must be falsified";
    }

    ++stats.conflicts;

    int dcnt = 0;
    b.clear();
    for (const auto &ll : clauses_[cc]) {
      const int li = (~ll).ID();
      const int ai = li >> 1;
      if (stamp[ai] < latestStamp) {
        stamp[ai] = latestStamp;
        heap.Inc(ai);
        if (level[ai] == d) {
          ++dcnt;
        } else if (level[ai] > 0) {
          b.push_back(ll);
        }
      }
    }
    for (size_t tt = L.size() - 1;; --tt) {
      // We only consider stamped literals.
      if (stamp[L[tt] >> 1] == latestStamp) {
        // When there's only one literal left from level d, we complete the
        // learned claused.
        if (dcnt == 1) {
          b.push_back(~Lit(L[tt]));
          std::swap(b[0], b.back());
          break;
        }
        CHECK(reason[L[tt]] != -1)
            << "reasons during clause learning cannot be decisions: t'=" << tt
            << " L[" << tt << "]=" << L[tt] << TrailString();
        for (const auto &ll : clauses_[reason[L[tt]]]) {
          const int li = (~ll).ID();
          const int ai = li >> 1;
          if (stamp[ai] < latestStamp) {
            stamp[ai] = latestStamp;
            heap.Inc(ai);
            if (level[ai] == d) {
              ++dcnt;
            } else if (level[ai] > 0) {
              b.push_back(ll);
            }
          }
        }
        --dcnt;
      }
    }

    // Simplify.
    // @see: 7.2.2.2 - exercise 257, p155
    for (size_t i = 1; i < b.size();) {
      if (IsRedundant(b, b[i])) {
        std::swap(b[i], b.back());
        b.pop_back();
      } else {
        ++i;
      }
    }

    // Trivial clause.
    // @see: 7.2.2.2 - exercise 23, p156
    if (d < b.size()) {
      b.clear();
      for (size_t i = d; i >= 1; --i) {
        b.push_back(~Lit(L[lloc[i]]));
      }
    }

    CHECK(level[b[0].VID()] == d)
        << "l'=" << ToString(b[0]) << " should be on level d=" << d
        << " but it is on level " << level[b[0].VID()];
    for (size_t i = 1; i < b.size(); ++i) {
      CHECK(level[b[i].VID()] < d)
          << "there must be a single literal in level d in clause ("
          << ToString(b) << ")" << TrailString();
    }

    // Calculate the backjump level.
    dd = 0;
    for (const auto &ll : b) {
      if (level[ll.VID()] < d) {
        dd = std::max(dd, level[ll.VID()]);
      }
    }
    CHECK(dd < d) << "backjump level d'=" << dd
                  << " should be below current level d=" << d;
  }

C8: // Backjump.
  while (L.size() > lloc[dd + 1]) {
    l = L.back();
    const int k = l >> 1;
    if (reason[l] != -1) {
      // We need to make sure clauses don't end up with falsified watchees.
      // This can probably be improved but I need to think about it.
      Clause &c = clauses_[reason[l]];
      const bool falsified =
          (~c[0] != b[0] && IsFalse(c[0]) && level[c[0].VID()] <= dd) ||
          (~c[1] != b[0] && IsFalse(c[1]) && level[c[1].VID()] <= dd);
      if (falsified) {
        w.Forget(reason[l]);
        for (size_t off = 0; off < 2; ++off) {
          if (~c[off] != b[0] && IsFalse(c[off]) && level[c[off].VID()] <= dd) {
            for (size_t i = 1 + off; i < c.size(); ++i) {
              if (!IsFalse(c[i]) || level[c[i].VID()] > dd) {
                std::swap(c[off], c[i]);
                break;
              }
            }
          }
        }
        w.Watch(reason[l]);
      }
      used[reason[l]] = 0;
      reason[l] = -1;
    }
    old[k] = val[k];
    val[k] = -1;
    level[k] = -1;
    if (!heap.Contains(k)) {
      heap.Push(k);
    }
    L.pop_back();
  }
  g = L.size();
  d = dd;
  LOG << "C8: backjump to d=" << d << " and G=" << g;

C9: // Learn.

  const int ll = b[0].ID();
  const int k = ll >> 1;

  // Check immediate subsumption.
  // @see: 7.2.2.2 - exercise 271, p156
  const bool subsumes =
      std::all_of(b.begin(), b.end(),
                  [&](const Lit &lit) {
                    return learnedStamp[lit.ID()] == latestStamp - 1;
                  }) &&
      !used[static_cast<int>(clauses_.size() - 1)];
  if (subsumes) {
    LOG << "C9: learned clause (" << ToString(b) << "), immediately subsuming ("
        << ToString(clauses_.back()) << ")";

    --stats.learnedClauses;
    stats.clauseLength -= static_cast<int64_t>(clauses_.back().size());

    w.Forget(static_cast<int>(clauses_.size() - 1));
    clauses_.pop_back();
    used.pop_back();
  } else {
    LOG << "C9: learned clause (" << ToString(b) << ")";
  }

  // Update learned stamps.
  for (const auto &lit : b) {
    learnedStamp[lit.ID()] = latestStamp;
  }
  ++latestStamp;
  // Add new clause.
  clauses_.push_back(b);
  used.push_back(ll);
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
  agility = agility - (agility >> 13) + (((old[k] - val[k]) & 1) << 19);
  // Update watches.
  w.Watch(static_cast<int>(clauses_.size() - 1));
  // Update stats.
  ++stats.learnedClauses;
  stats.clauseLength += static_cast<int64_t>(b.size());

  goto C3;
}

std::pair<Result, std::vector<Assignment>> C::SolveAll() {
  COMMENT << "this solver does not support listing all satisfying assignments";
  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
