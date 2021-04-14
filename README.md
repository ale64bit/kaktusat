kaktusat
========

A playground for learning about satisfiability and SAT solvers.

Basic commands
--------------

Run all tests:
```
bazel test //test:all
```

Run main solver binary on CNF instance (here, `D` is the algorithm used):
```
bazel run -c opt //main -- D /path/to/instance.cnf
```

Run batch against all instances in a directory:
```
bazel run -c opt //benchmark:batch -- D /path/to/instance/dir/
```

Main packages
-------------
* [solver](solver/): solver interface and utilities to specify instances
* [solver/algorithm](solver/algorithm/): contains solver algorithm implementations
* [solver/encoder](solver/encoder/): contains instance encoders (e.g. cardinality, coloring, circuits, DIMACS, etc.)
* [solver/transform](solver/transform/): contains instance transforms (currently 3SAT and monotone reductions)

Demos
-----

* [factor](demo/factor.cpp): encodes a factorization problem. Number to factorize is given as a program argument.
* [sudoku](demo/sudoku.cpp): encodes a sudoku problem. Sudoku to solve is read from stdin.

Misc
----
[doc](doc) contains some of my notes on exercises from 7.2.2.2 and some flowcharts for the algorithms.
