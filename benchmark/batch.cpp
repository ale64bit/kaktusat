#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <string>

#include "solver/algorithm/algorithm.h"
#include "solver/encoder/dimacs.h"

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "usage: batch <algorithm> <dir>" << std::endl;
    return 0;
  }

  std::string solverID(argv[1]);
  std::string dir(argv[2]);

  std::map<std::string, std::unique_ptr<solver::Solver>> solvers;
  solvers["A"] = std::make_unique<solver::algorithm::A>();
  solvers["A2"] = std::make_unique<solver::algorithm::A2>();
  solvers["B"] = std::make_unique<solver::algorithm::B>();
  solvers["C"] = std::make_unique<solver::algorithm::C>();
  solvers["D"] = std::make_unique<solver::algorithm::D>();
  solvers["I0"] = std::make_unique<solver::algorithm::I0>();

  if (solvers.count(solverID) == 0) {
    std::cout << "unknown algorithm: " << solverID << std::endl;
    return 1;
  }

  auto &solver = *solvers[solverID];

  int cnt = 1;
  int sat = 0;
  int unsat = 0;
  double totalTime = 0;
  double minTime = std::numeric_limits<double>::max();
  double maxTime = std::numeric_limits<double>::min();
  for (const auto &e : fs::recursive_directory_iterator(dir)) {
    if (!e.is_regular_file() || e.path().extension() != ".cnf") {
      continue;
    }
    solver.Reset();
    auto err = solver::encoder::FromDimacsFile(solver, e.path());
    if (!err.empty()) {
      std::cerr << "reading instance: " << err << std::endl;
      return 1;
    }
    auto start = std::chrono::system_clock::now();
    auto [res, sol] = solver.Solve();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::string errMsg;
    switch (res) {
    case solver::Result::kSAT:
      if (!solver.Verify(sol, &errMsg)) {
        std::cerr << cnt << ": " << e.path() << ": bad verify: " << errMsg
                  << std::endl;
        return 1;
      }
      ++sat;
      break;
    case solver::Result::kUNSAT:
      ++unsat;
      break;
    case solver::Result::kUnknown:
      std::cerr << cnt << ": " << e.path() << ": unknown" << std::endl;
      return 1;
    }
    std::cout << cnt << ": " << e.path() << ": "
              << (res == solver::Result::kSAT ? "SAT" : "UNSAT") << " in "
              << std::fixed << std::setprecision(3) << diff.count() << " sec"
              << '\n';
    totalTime += diff.count();
    minTime = std::min(minTime, diff.count());
    maxTime = std::max(maxTime, diff.count());
    ++cnt;
  }
  std::cout << "all ok in " << std::fixed << std::setprecision(3) << totalTime
            << " sec" << '\n';

  if (sat == 0) {
    std::cout << "\tall UNSAT" << '\n';
  } else if (unsat == 0) {
    std::cout << "\tall SAT" << '\n';
  } else {
    std::cout << "\t#SAT=" << sat << " #UNSAT=" << unsat << '\n';
  }

  std::cout << "\tavg_time = " << (totalTime / cnt) << " sec\n"
            << "\tmin_time = " << minTime << " sec\n"
            << "\tmax_time = " << maxTime << " sec" << std::endl;

  return 0;
}
