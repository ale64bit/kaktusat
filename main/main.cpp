#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "solver/algorithm/a.h"
#include "solver/algorithm/a2.h"
#include "solver/algorithm/analyze.h"
#include "solver/algorithm/b.h"
#include "solver/algorithm/d.h"
#include "solver/algorithm/nop.h"
#include "solver/algorithm/z.h"
#include "solver/builder/cardinality.h"
#include "solver/builder/dimacs.h"
#include "solver/builder/langford.h"
#include "solver/builder/simple.h"
#include "solver/builder/waerden.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "usage: main <algorithm> <instance.cnf>" << std::endl;
    return 0;
  }

#ifdef NDEBUG
  std::ofstream devNull("/dev/null");
  std::clog.rdbuf(devNull.rdbuf());
#endif

  std::string solverID(argv[1]);
  std::string path(argv[2]);

  std::map<std::string, std::unique_ptr<solver::Solver>> solvers;
  solvers["A"] = std::make_unique<solver::algorithm::A>();
  solvers["A2"] = std::make_unique<solver::algorithm::A2>();
  solvers["B"] = std::make_unique<solver::algorithm::B>();
  solvers["D"] = std::make_unique<solver::algorithm::D>();
  solvers["NOP"] = std::make_unique<solver::algorithm::Nop>();
  solvers["Z"] = std::make_unique<solver::algorithm::Z>();
  solvers["?"] = std::make_unique<solver::algorithm::Analyze>();

  if (solvers.count(solverID) == 0) {
    std::cout << "unknown algorithm: " << solverID << std::endl;
    return 1;
  }

  auto &solver = *solvers[solverID];

  {
    std::cout << "Reading instance from DIMACS file..." << std::endl;
    auto start = std::chrono::system_clock::now();
    auto err = solver::builder::FromDimacsFile(solver, path);
    if (!err.empty()) {
      std::cout << "Reading instance: " << err << std::endl;
      return 1;
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Instance built in " << std::fixed << std::setprecision(3)
              << diff.count() << " secs" << std::endl;
  }

  {
    std::cout << "Solving with Algorithm " << solverID << "..." << std::endl;
    auto start = std::chrono::system_clock::now();
    auto [res, sol] = solver.Solve();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Finished in " << std::fixed << std::setprecision(3)
              << diff.count() << " secs" << std::endl;

    std::cout << "Instance: " << solver.ToString() << '\n';
    std::string errMsg;
    switch (res) {
    case solver::Result::kSAT:
      std::cout << "Result: SAT" << '\n';
      std::cout << "Solution: [" << solver.ToString(sol) << "]" << '\n';
      if (!solver.Verify(sol, &errMsg)) {
        std::cout << "Verify: " << errMsg << '\n';
      } else {
        std::cout << "Verify: OK" << '\n';
      }
      break;
    case solver::Result::kUNSAT:
      std::cout << "Result: UNSAT" << '\n';
      break;
    case solver::Result::kUnknown:
      std::cout << "Result: UNKNOWN" << '\n';
      break;
    }
  }

  return 0;
}
