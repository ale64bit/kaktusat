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
  double total = 0;
  double minTime = std::numeric_limits<double>::max();
  double maxTime = std::numeric_limits<double>::min();
  for (const auto &p : fs::directory_iterator(dir)) {
    if (p.path().extension() != ".cnf") {
      continue;
    }
    solver.Reset();
    auto err = solver::encoder::FromDimacsFile(solver, p.path());
    if (!err.empty()) {
      std::cerr << "reading instance: " << err << std::endl;
      return 1;
    }
    auto start = std::chrono::system_clock::now();
    auto [res, sol] = solver.Solve();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    if (res != solver::Result::kSAT) {
      std::cerr << cnt << ": " << p.path() << ": bad result" << '\n';
      return 1;
    }
    std::string errMsg;
    if (!solver.Verify(sol, &errMsg)) {
      std::cerr << cnt << ": " << p.path() << ": bad verify: " << errMsg
                << '\n';
      return 1;
    }
    std::cout << cnt << ": " << p.path() << ": ok in " << std::fixed
              << std::setprecision(3) << diff.count() << " sec" << '\n';
    total += diff.count();
    minTime = std::min(minTime, diff.count());
    maxTime = std::max(maxTime, diff.count());
    ++cnt;
  }
  std::cout << "all ok in " << std::fixed << std::setprecision(3) << total
            << " sec" << '\n'
            << "\tavg_time = " << (total / cnt) << " sec\n"
            << "\tmin_time = " << minTime << " sec\n"
            << "\tmax_time = " << maxTime << " sec" << std::endl;

  return 0;
}
