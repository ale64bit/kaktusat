#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "solver/algorithm/a.h"
#include "solver/algorithm/a2.h"
#include "solver/algorithm/b.h"
#include "solver/algorithm/d.h"
#include "solver/builder/dimacs.h"

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "usage: stress <algorithm> <dir>" << std::endl;
    return 0;
  }

#ifdef NDEBUG
  std::ofstream devNull("/dev/null");
  std::clog.rdbuf(devNull.rdbuf());
#endif

  std::string solverID(argv[1]);
  std::string dir(argv[2]);

  std::map<std::string, std::unique_ptr<solver::Solver>> solvers;
  solvers["A"] = std::make_unique<solver::algorithm::A>();
  solvers["A2"] = std::make_unique<solver::algorithm::A2>();
  solvers["B"] = std::make_unique<solver::algorithm::B>();
  solvers["D"] = std::make_unique<solver::algorithm::D>();

  if (solvers.count(solverID) == 0) {
    std::cout << "unknown algorithm: " << solverID << std::endl;
    return 1;
  }

  auto &solver = *solvers[solverID];

  int cnt = 1;
  for (const auto &p : fs::directory_iterator(dir)) {
    if (p.path().extension() != ".cnf") {
      continue;
    }
    solver.Reset();
    auto err = solver::builder::FromDimacsFile(solver, p.path());
    if (!err.empty()) {
      std::cerr << "Reading instance: " << err << std::endl;
      return 1;
    }
    auto [res, sol] = solver.Solve();
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
    std::cout << cnt << ": " << p.path() << " : ok " << '\n';
    ++cnt;
  }
  std::cout << "all ok" << std::endl;
}
