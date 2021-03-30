#include <chrono>
#include <iomanip>
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
#include "solver/builder/dimacs.h"
#include "util/log.h"

int main(int argc, char *argv[]) {
  util::InitLogging();

  if (argc != 3) {
    std::cout << "usage: main <algorithm> <instance.cnf>" << std::endl;
    return 0;
  }

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
    std::cerr << "unknown algorithm: " << solverID;
    return 1;
  }

  auto &solver = *solvers[solverID];

  {
    LOG << "Reading instance from DIMACS file...";
    auto start = std::chrono::system_clock::now();
    auto err = solver::builder::FromDimacsFile(solver, path);
    if (!err.empty()) {
      LOG << "Reading instance: " << err;
      return 1;
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    LOG << "Instance built in " << std::fixed << std::setprecision(3)
        << diff.count() << " secs";
  }

  {
    LOG << "Solving with Algorithm " << solverID << "...";
    auto start = std::chrono::system_clock::now();
    auto [res, sol] = solver.Solve();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    LOG << "Finished in " << std::fixed << std::setprecision(3) << diff.count()
        << " secs";

    LOG << "Instance: " << solver.ToString();
    std::string errMsg;
    switch (res) {
    case solver::Result::kSAT:
      LOG << "Result: SAT";
      LOG << "Solution: [" << solver.ToString(sol) << "]";
      if (!solver.Verify(sol, &errMsg)) {
        LOG << "Verify: " << errMsg;
      } else {
        LOG << "Verify: OK";
      }
      break;
    case solver::Result::kUNSAT:
      LOG << "Result: UNSAT";
      break;
    case solver::Result::kUnknown:
      LOG << "Result: UNKNOWN";
      break;
    }
  }

  return 0;
}
