#include <chrono>
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
#include "solver/algorithm/i0.h"
#include "solver/algorithm/nop.h"
#include "solver/algorithm/z.h"
#include "solver/builder/dimacs.h"
#include "util/log.h"

constexpr size_t kValuesPerLine = 10;

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
  solvers["I0"] = std::make_unique<solver::algorithm::I0>();
  solvers["NOP"] = std::make_unique<solver::algorithm::Nop>();
  solvers["Z"] = std::make_unique<solver::algorithm::Z>();
  solvers["?"] = std::make_unique<solver::algorithm::Analyze>();

  if (solvers.count(solverID) == 0) {
    std::cerr << "unknown algorithm: " << solverID;
    return 1;
  }

  auto &solver = *solvers[solverID];

  {
    COMMENT << "reading instance from DIMACS file: " << path;
    auto start = std::chrono::system_clock::now();
    auto err = solver::builder::FromDimacsFile(solver, path);
    if (!err.empty()) {
      LOG << "error while reading instance: " << err;
      return 1;
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    COMMENT << "instance read in " << std::fixed << std::setprecision(3)
            << diff.count() << " secs";
  }

  {
    COMMENT << "solving with Algorithm " << solverID;
    auto start = std::chrono::system_clock::now();
    auto [res, sol] = solver.Solve();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    COMMENT << "finished in " << std::fixed << std::setprecision(3)
            << diff.count() << " secs";

    LOG << "instance: " << solver.ToString();
    std::string errMsg;
    switch (res) {
    case solver::Result::kSAT:
      RESULT << "SATISFIABLE";
      for (size_t i = 0; i < sol.size(); i += kValuesPerLine) {
        VALUES << solver.ToString(
            std::vector(sol.begin() + i,
                        sol.begin() + std::min(sol.size(), i + kValuesPerLine)),
            " ", true);
      }
      VALUES << 0;
      LOG << "solution: [" << solver.ToString(sol) << "]";
      if (!solver.Verify(sol, &errMsg)) {
        LOG << "verify: " << errMsg;
      } else {
        LOG << "verify: OK";
      }
      break;

    case solver::Result::kUNSAT:
      RESULT << "UNSATISFIABLE";
      break;

    case solver::Result::kUnknown:
      RESULT << "UNKNOWN";
      break;
    }
  }

  COMMENT << "done";
  return 0;
}
