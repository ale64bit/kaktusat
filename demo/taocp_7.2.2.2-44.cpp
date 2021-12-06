// Solution to TAOCP 7.2.2.2 - exercise 44, p136
#include <bitset>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>

#include "solver/algorithm/algorithm.h"
#include "solver/encoder/cardinality.h"
#include "solver/encoder/encoder.h"
#include "solver/encoder/multiply.h"
#include "util/log.h"

// Utility function to get an idea of what kind of values are possible.
[[maybe_unused]] void randomSearch() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> distrib(
      0, std::numeric_limits<uint32_t>::max());

  constexpr int passes = 2;
  int ans = 0;
  int a = 0;
  int b = 0;
  for (int i = 0;; ++i) {
    uint64_t x = 0;
    uint64_t y = 0;
    for (int j = 0; j < passes; ++j) {
      x |= distrib(gen);
      y |= distrib(gen);
    }
    uint64_t z = x * y;

    int cnt = std::bitset<32>(x).count() + std::bitset<32>(y).count() +
              std::bitset<64>(z).count();
    if (cnt > ans) {
      ans = cnt;
      a = std::bitset<32>(x).count();
      b = std::bitset<32>(y).count();
      std::cout << "max=" << ans << " a=" << a << " b=" << b << std::endl;
    }
  }
  std::cout << "max=" << ans << " a=" << a << " b=" << b << std::endl;
}

int main() {
  util::InitLogging();

  using namespace solver;

  for (int goal = 4 * 32;; --goal) {
    std::cout << "goal=" << goal << std::endl;
    bool any = false;
    for (int a = 32; a >= 1 && !any; --a) {
      for (int b = 32; b >= a && !any; --b) {
        int c = goal - a - b;
        if (c > 64) {
          continue;
        }
        algorithm::Default solver;
        encoder::Multiply(solver, 32, 32);
        std::vector<Lit> x;
        std::vector<Lit> y;
        std::vector<Lit> z;
        for (int i = 1; i <= 32; ++i) {
          x.push_back(solver.GetVar("x" + std::to_string(i)));
          y.push_back(solver.GetVar("y" + std::to_string(i)));
        }
        for (int i = 1; i <= 64; ++i) {
          z.push_back(solver.GetVar("z" + std::to_string(i)));
        }

        encoder::Exactly(solver, x, a);
        encoder::Exactly(solver, y, b);
        encoder::Exactly(solver, z, c);

        std::cout << "\ta=" << a << " b=" << b << " c=" << c
                  << ": solving with n=" << solver.NumVars()
                  << " m=" << solver.NumClauses() << "...";
        std::cout.flush();
        auto [res, sol] = solver.Solve();
        if (res == Result::kSAT) {
          any = true;
          std::cout << "SAT" << std::endl;
          uint32_t x = 0;
          uint32_t y = 0;
          for (const auto &l : sol) {
            if (l.IsPos() && !solver.IsTemp(l.V())) {
              std::string name = solver.NameOf(l.V());
              int idx = std::atoi(name.data() + 1);
              if (name[0] == 'x') {
                x |= (1u << (idx - 1));
              } else if (name[0] == 'y') {
                y |= (1u << (idx - 1));
              }
            }
          }
          std::cout << "\t\tx=" << x << " y=" << y << std::endl;
          return 0;
        } else {
          std::cout << "UNSAT" << std::endl;
        }
      }
    }
    std::cout << "goal=" << goal << " is " << (any ? "possible" : "impossible")
              << std::endl;
  }

  /*
   * All values v(x) + v(y) + v(xy) >= 113 are UNSAT.
   *
   * Example for 112:
   *     4290904001 * 4290637759 = 18410714726934773759 (22,28,62)
   *
   * Another large example for 110:
   *     4277071599 * 4026531615 = 17221764012992102385 (26,28,56)
   *
   * Thus the answer is 112.
   */

  return 0;
}
