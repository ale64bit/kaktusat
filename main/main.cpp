#include "solver/algorithm/A.h"
#include "solver/algorithm/Z.h"
#include "solver/builder/dimacs.h"
#include "solver/builder/simple.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "usage: main <algorithm> <instance.cnf>" << std::endl;
    return 0;
  }

#ifdef NDEBUG
  std::ofstream devNull("/dev/null");
  std::clog.rdbuf(devNull.rdbuf());
#endif

  char solverID = argv[1][0];
  std::string path(argv[2]);

  std::map<char, std::unique_ptr<solver::Solver>> solvers;
  solvers['A'] = std::make_unique<solver::algorithm::A>();
  solvers['Z'] = std::make_unique<solver::algorithm::Z>();

  if (solvers.count(solverID) == 0) {
    std::cout << "unknown algorithm: " << solverID << std::endl;
    return 1;
  }

  auto solver = solvers[solverID].get();

  {
    std::cout << "Reading instance from dimacs file..." << std::endl;
    auto start = std::chrono::system_clock::now();
    auto err = solver::builder::FromDimacsFile(path, *solver);
    if (!err.empty()) {
      std::cout << "Reading instance: " << err << std::endl;
      return 1;
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Instance built in " << std::fixed << std::setprecision(3)
              << diff.count() << " secs" << std::endl;
    std::cout << "  N=" << solver->NumVars() << " M=" << solver->NumClauses()
              << std::endl;
  }

  {
    std::cout << "Solving with Algorithm " << solverID << "..." << std::endl;
    auto start = std::chrono::system_clock::now();
    auto [res, sol] = solver->Solve();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Finished in " << std::fixed << std::setprecision(3)
              << diff.count() << " secs" << std::endl;

    std::cout << "Instance: " << solver->ToString() << '\n';
    std::string errMsg;
    switch (res) {
    case solver::Result::kSAT:
      std::cout << "Result: SAT" << '\n';
      std::cout << "Solution: [" << solver->ToString(sol) << "]" << '\n';
      if (!solver->Verify(sol, &errMsg)) {
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
