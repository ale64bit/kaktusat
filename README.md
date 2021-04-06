kaktusat
========

A playground for learning about satisfiability and SAT solvers.

Basic commands
--------------

Run all tests:
```
bazel test //test:all
```

Run solver on CNF instance:
```
bazel run -c opt //main -- D /path/to/instance.cnf
```
