#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "solver/algorithm/a.h"
#include "solver/algorithm/a2.h"
#include "solver/algorithm/b.h"
#include "solver/algorithm/c.h"
#include "solver/algorithm/d.h"
#include "solver/algorithm/i0.h"
#include "solver/algorithm/z.h"
#include "solver/encoder/anti_maximal_element.h"
#include "solver/encoder/cardinality.h"
#include "solver/encoder/coloring.h"
#include "solver/encoder/encoder.h"
#include "solver/encoder/factor.h"
#include "solver/encoder/langford.h"
#include "solver/encoder/mutilated_chessboard.h"
#include "solver/encoder/pigeonhole.h"
#include "solver/encoder/sample.h"
#include "solver/encoder/waerden.h"

using SolverFn = std::function<std::unique_ptr<solver::Solver>()>;
using EncoderFn = std::function<void(solver::Solver &)>;
using SolverTestParam =
    std::tuple<SolverFn, std::pair<const char *, EncoderFn>>;

class SATTest : public testing::TestWithParam<SolverTestParam> {};
class UNSATTest : public testing::TestWithParam<SolverTestParam> {};

TEST_P(SATTest, Solve) {
  auto [solverFn, encoderP] = GetParam();
  auto [id, encoderFn] = encoderP;
  auto solver = solverFn();
  encoderFn(*solver);
  auto [res, sol] = solver->Solve();
  EXPECT_EQ(res, solver::Result::kSAT);
  EXPECT_TRUE(solver->Verify(sol));
}

TEST_P(UNSATTest, Solve) {
  auto [solverFn, encoderP] = GetParam();
  auto [id, encoderFn] = encoderP;
  auto solver = solverFn();
  encoderFn(*solver);
  auto [res, sol] = solver->Solve();
  EXPECT_EQ(res, solver::Result::kUNSAT);
  EXPECT_TRUE(sol.empty());
}

namespace enc = solver::encoder;
namespace graph = solver::encoder::graph;

INSTANTIATE_TEST_SUITE_P(
    Solver, SATTest,
    testing::Combine(
        // Solvers
        testing::Values(
            []() { return std::make_unique<solver::algorithm::A>(); },
            []() { return std::make_unique<solver::algorithm::A2>(); },
            []() { return std::make_unique<solver::algorithm::B>(); },
            []() { return std::make_unique<solver::algorithm::C>(); },
            []() { return std::make_unique<solver::algorithm::D>(); },
            []() { return std::make_unique<solver::algorithm::I0>(); }),
        // SAT instances
        testing::Values(
            std::make_pair("Unit", [](solver::Solver &s) { enc::Unit(s); }),
            std::make_pair("Tautology",
                           [](solver::Solver &s) { enc::Tautology(s); }),
            std::make_pair("Rprime", [](solver::Solver &s) { enc::Rprime(s); }),
            std::make_pair("WaerdenI3J3N8",
                           [](solver::Solver &s) { enc::Waerden(s, 3, 3, 8); }),
            std::make_pair("Langford3",
                           [](solver::Solver &s) { enc::Langford(s, 3); }),
            std::make_pair("Langford4",
                           [](solver::Solver &s) { enc::Langford(s, 4); }),
            std::make_pair("Langford7",
                           [](solver::Solver &s) { enc::Langford(s, 7); }),
            std::make_pair("PetersenC3",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 3, graph::Petersen());
                           }),
            std::make_pair("McGregor3",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 4, graph::McGregor3());
                           }),
            std::make_pair("FlowerSnarkC2Q4",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 2, graph::FlowerSnark(4));
                           }),
            std::make_pair("FlowerSnarkC3Q5",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 3, graph::FlowerSnark(5));
                           }),
            std::make_pair("FlowerSnarkLineC3Q4",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 3, graph::FlowerSnarkLine(4));
                           }),
            std::make_pair("FlowerSnarkLineC4Q5",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 4, graph::FlowerSnarkLine(5));
                           }),
            std::make_pair("AtLeastAtMost",
                           [](solver::Solver &s) {
                             std::vector<solver::Lit> x;
                             for (int i = 1; i <= 7; ++i) {
                               x.push_back(s.NewVar("x" + std::to_string(i)));
                             }
                             enc::AtLeast(s, x, 3);
                             enc::AtMost(s, x, 4);
                           }),
            std::make_pair("FactorX2Y3N21",
                           [](solver::Solver &s) {
                             enc::Factor(s, 2, 3, 21);
                           }))),
    [](const testing::TestParamInfo<SolverTestParam> &info) {
      auto solverFn = std::get<0>(info.param);
      auto id = std::get<1>(info.param).first;
      return solverFn()->ID() + "with" + id;
    });

INSTANTIATE_TEST_SUITE_P(
    Solver, UNSATTest,
    testing::Combine(
        // Solvers
        testing::Values(
            []() { return std::make_unique<solver::algorithm::A>(); },
            []() { return std::make_unique<solver::algorithm::A2>(); },
            []() { return std::make_unique<solver::algorithm::B>(); },
            []() { return std::make_unique<solver::algorithm::C>(); },
            []() { return std::make_unique<solver::algorithm::D>(); },
            []() { return std::make_unique<solver::algorithm::I0>(); }),
        // UNSAT instances
        testing::Values(
            std::make_pair("Contradiction",
                           [](solver::Solver &s) { enc::Contradiction(s); }),
            std::make_pair("R", [](solver::Solver &s) { enc::R(s); }),
            std::make_pair("WaerdenI3J3N9",
                           [](solver::Solver &s) { enc::Waerden(s, 3, 3, 9); }),
            std::make_pair("Langford2",
                           [](solver::Solver &s) { enc::Langford(s, 2); }),
            std::make_pair("Langford5",
                           [](solver::Solver &s) { enc::Langford(s, 5); }),
            std::make_pair("Langford6",
                           [](solver::Solver &s) { enc::Langford(s, 6); }),
            std::make_pair("PetersenC2",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 2, graph::Petersen());
                           }),
            std::make_pair("McGregorC3",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 3, graph::McGregor3());
                           }),
            std::make_pair("FlowerSnarkC2Q5",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 2, graph::FlowerSnark(5));
                           }),
            std::make_pair("FlowerSnarkC2Q4",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 2, graph::FlowerSnarkLine(4));
                           }),
            std::make_pair("FlowerSnarkLineC3Q5",
                           [](solver::Solver &s) {
                             enc::Coloring(s, 3, graph::FlowerSnarkLine(5));
                           }),
            std::make_pair("AtLeastAtMost",
                           [](solver::Solver &s) {
                             std::vector<solver::Lit> x;
                             for (int i = 1; i <= 7; ++i) {
                               x.push_back(s.NewVar("x" + std::to_string(i)));
                             }
                             solver::encoder::AtLeast(s, x, 4);
                             solver::encoder::AtMost(s, x, 3);
                           }),
            std::make_pair("AntiMaximalElement",
                           [](solver::Solver &s) {
                             enc::AntiMaximalElement(s, 3);
                           }),
            std::make_pair("Pigeonhole3",
                           [](solver::Solver &s) { enc::Pigeonhole(s, 3); }),
            std::make_pair("Pigeonhole4",
                           [](solver::Solver &s) { enc::Pigeonhole(s, 4); }),
            std::make_pair("MutilatedChessBoard4",
                           [](solver::Solver &s) {
                             enc::MutilatedChessboard(s, 4);
                           }),
            std::make_pair("MutilatedChessBoard5",
                           [](solver::Solver &s) {
                             enc::MutilatedChessboard(s, 5);
                           }),
            std::make_pair("FactorX2Y3N19",
                           [](solver::Solver &s) {
                             enc::Factor(s, 2, 3, 19);
                           }))),
    [](const testing::TestParamInfo<SolverTestParam> &info) {
      auto solverFn = std::get<0>(info.param);
      auto id = std::get<1>(info.param).first;
      return solverFn()->ID() + "with" + id;
    });
