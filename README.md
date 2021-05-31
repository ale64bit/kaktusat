kaktusat
========

Introduction
------------

This is a playground for learning about several topics that interests me, including [satisfiability](https://en.wikipedia.org/wiki/Boolean_satisfiability_problem), [SAT solving](https://en.wikipedia.org/wiki/Boolean_satisfiability_problem#Algorithms_for_solving_SAT), [proof theory](https://en.wikipedia.org/wiki/Proof_theory) and [mathematical logic](https://en.wikipedia.org/wiki/Mathematical_logic) in general. Currently, it consists of the following:

* [solver interface](solver/solver.h): a SAT solver interface with basic functionality such as creating new variables, adding clauses, solving and verifying instances. This interface is akin to those used in well-known solvers such as [[1]](#1).

* [encoders](solver/encoder): encoders encode an instance of a problem into a satisfiability instance that can be readily used with a SAT solver. Such problems include [cardinality contraints](solver/encoder/cardinality.h), [graph coloring](solver/encoder/coloring.h), [circuits](solver/encoder/circuit.h), etc. Encoders are not mutually exclusive, e.g. circuit encoders can be used incrementally for build an integer factorization instance. An encoder to read instances stored in external files in the [DIMACS](http://www.satcompetition.org/2009/format-benchmarks2009.html) format is also included.

* [transforms](solver/transform): transforms take an existing instance and transform it to an equivalent instance with desirable properties. For example, the [sat3](solver/transform/sat3.h) transform takes an arbitrary k-CNF instance and transforms it to an equivalent 3-CNF instance.

* [algorithms](solver/algorithm): algorithms to actually solve SAT instances. Most of them are named and implemented following [[2]](#2) and are self-contained.

* [demos](demo): short programs illustrating the usage of the above.

* [repl](repl): a [REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) for exploring basic logic computations interactively.

As I study and learn new things, this organization might change and new items will be added. Additional notes on exercises from [[2]](#2) and diagrams can be found in [doc](doc). 

The following sections provide more detail on specific components.

Algorithms
----------

Currently the following algorithms are implemented (listed by their corresponding ID):

* [A](solver/algorithm/a.h): implements **Algorithm A** (*Satisfiability by backtracking*), as presented in [[2]](#2), page 28.
* [A2](solver/algorithm/a2.h): small variation of **Algorithm A**.
* [Analyze](solver/algorithm/analyze.h): not really a solver, but rather an analyzer of instances, printing interesting information such as clause length statistics and redundant literals.
* [B](solver/algorithm/b.h): implements **Algorithm B** (*Satisfiability by watching*), as presented in [[2]](#2), page 31.
* [C](solver/algorithm/c.h): implements **Algorithm C** (*Satisfiability by CDCL*), as presented in [[2]](#2), page 68.
* [D](solver/algorithm/d.h): implements **Algorithm D** (*Satisfiability by cyclic DPLL*), as presented in [[2]](#2), page 33.
* [I0](solver/algorithm/i0.h): implements **Algorithm I** (*Satisfiability by clause learning*), as presented in [[2]](#2), page 61. Or rather, a straightforward implementation of the algorithm description, since it's a family of algorithms rather than a specific one.
* [NOP](solver/algorithm/nop.h): a dummy solver that always returns `UNKNOWN` as result. It's useful for testing properties that do not need an actual solver, such as encoders and transformers.
* [Z](solver/algorithm/z.h): the slowest solver ever. Literally tries every assignment.

Encoders
--------

These are some of the provided encoders:

* [basic](solver/encoder/encoder.h): encoders for basic concepts: unit clauses, tautologies, contradictions.
* [cardinality](solver/encoder/cardinality.h): encodes cardinality constraints such as *"at most two of these variables can be true"*. It employs techniques described in [[2]](#2), [[3]](#3) and [[4]](#4), among others. They serve as the basis of many other encoders and transforms.
* [circuit](solver/encoder/circuit.h): encodes common circuit functions, such as `AND`, `OR` and `NOT` gates. Serves as the basis for e.g. multiplication and factoring encoders. See [Tseytin transformation](https://en.wikipedia.org/wiki/Tseytin_transformation) and [[5]](#5).
* [dimacs](solver/encoder/dimacs.h): reads an instance from an external file in DIMACS CNF format.
* [rand](solver/encoder/rand.h): generates a random k-CNF instance on N variables and M clauses.

The full list can be examined [here](solver/encoder).

Basic usage
-----------

Most of the code is written in C++ and uses the [Bazel](https://bazel.build/) build system.

The solver can be used from a provided command-line application that takes the algorithm ID to use and the path to a DIMACS file in CNF format. For example:
```bash
bazel run -c opt //main -- C /path/to/instance.cnf
```
will try to solve the instance contained in the file `/path/to/instance.cnf` using Algorithm C (see [[2]](#2)).

The solver can also be used programmatically. Here's a small example with comments:
```cpp
#include <iostream>

#include "solver/solver.h"      // general solver interface
#include "solver/algorithm/c.h" // specific algorithm to use
#include "util/log.h"           // utilities for logging

int main() {
  util::InitLogging();

  // Create an instance of the solver.
  solver::algorithm::C solver;

  // Create a few new variables. Variables can be named arbitrarily.
  auto x1 = solver.NewVar("x1");
  auto x2 = solver.NewVar("x2");
  auto x3 = solver.NewVar("x3");
  auto x4 = solver.NewVar("x4");

  // Add a few clauses. Variables are implicitly converted to literals.
  // The ~ operator negates a literal.
  solver.AddClause({x1, x2, ~x3});
  solver.AddClause({x2, x3, ~x4});
  solver.AddClause({x3, x4, x1});
  solver.AddClause({x4, ~x1, x2});
  solver.AddClause({~x1, ~x2, x3});
  solver.AddClause({~x2, ~x3, x4});
  solver.AddClause({~x3, ~x4, ~x1});

  // Actually solve the instance.
  auto [res, sol] = solver.Solve();
  if (res == solver::Result::kSAT) {
    // Print the solution using the original variable names.
    std::cout << "SAT: " << solver.ToString(sol) << std::endl;
  } else {
    std::cout << "UNSAT" << std::endl;
  }

  return 0;
}

```
This example recreates the satisfiable instance `R'` from [[2]](#2), page 4. It outputs:
```bash
SAT: ¬x1, x4, x2, x3
```

REPL
----

The REPL can be executed as follows:
```bash
bazel run -c opt //repl
```

It is useful for interactively exploring basic logic computations. The notation used mostly follows the one in [[6]](#6) and [[7]](#7).

![Demo](doc/img/repl_demo.svg)

Demos
-----

A couple of demos are provided in [demo](demo):

* [factor](demo/factor.cpp): encodes a factorization problem. Number to factorize is given as a program argument.
* [sudoku](demo/sudoku.cpp): encodes a sudoku problem. Sudoku to solve is read from stdin.

Development usage
-----------------

When developing, the following commands are handy.

For running all the tests:
```bash
bazel test //test:all
```

The current test suite includes unit tests for most algorithms and many of the encoders/transforms.

Additionally, a utility application for batch running a specific algorithm against all the CNF instance files in a directory is included:
```bash
bazel run -c opt //benchmark:batch -- D /path/to/instance/dir/
```
It will also verify results and output statistics about solve time.

References
----------

<a id="1">[1]</a>
 Eén, N., Sörensson, N.: An Extensible SAT-solver. In: Theory and Applications of Satisfiability Testing. pp. 502–518. Springer Berlin Heidelberg (2004)


<a id="2">[2]</a>
 Knuth, D. E.: The Art of Computer Programming, Volume 4, Fascicle 6: Satisfiability. Pearson Education (US) (2015)

<a id="3">[3]</a>
 Sinz, C.: Towards an Optimal CNF Encoding of Boolean Cardinality Constraints. In: Principles and Practice of Constraint Programming - CP 2005. pp. 827–831. Springer Berlin Heidelberg (2005)


<a id="4">[4]</a>
 Bailleux, O., Boufkhad, Y.: Efficient CNF Encoding of Boolean Cardinality Constraints. In: Principles and Practice of Constraint Programming – CP 2003. pp. 108–122. Springer Berlin Heidelberg (2003)


<a id="5">[5]</a>
 Tseytin, G. S.: On the complexity of derivation in propositional calculus. In: (ed.)Slisenko, A. O. Studies in Constructive Mathematics and Mathematical Logic. pp. 115–125. Springer (1969)

<a id="6">[6]</a>
 Krajícek, J.: Proof Complexity. Cambridge University Press (2019)


<a id="7">[7]</a>
 Buss, S.: Handbook of Proof Theory. Elsevier, New York (1998)

