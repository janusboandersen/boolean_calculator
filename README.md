# Boolean Calculator

- This calculator implements the base boolean operators _and_ (conjunction), _or_ (disjunction), _not_ (complement), as well as methods to derive _Shannon Cofactors_ and perform _tautology_-check on boolean expressions.
- The operators work on boolean expressions in Sum-of-Products (SOP) form, i.e. disjunctive normal form (DNF). 
- The calculator uses positional cube notation (PCN), and as such the core data structures are the `BooleanVariable` (as stand-alone), `Factor` (as used in a product term), `Cube` (product term), and `CubeList` (sum of products, complete boolean function or expression).

The unate recursive paradigm (URP) is used for computation. A heuristic for selecting which boolean variable to recurse on is also implemented.

## Unit tests
- Test cases for key/core behavior are in `test/testcases`.
- Also serve as examples of how the calculator can be used.
- Implemented and executable with Google Test. Coverage using lcov and gcov.

## Main app
- The main executable in `app/main.cpp` reads an SOP from each of the 5 `.pcn` files located in `data/UnateRecursiveComplement/in/`.
- For each SOP, the complement is computed and the result written to disk in the `.../out/` folder.

## Toolchain
- This is a C++17 project without any external library dependencies, so the code as stand-alone should work on most modern systems.
- Several supporting utilities are however targeted for Linux (Ubuntu 22.04), so the complete set of CMake build targets is likely to only be Linux-compatible.
- actions.py is used to invoke CMake.

## Improvement ideas

- This project is an example implementation of computational boolean algebra using PCN and URP, and as such, the results are not guaranteed to be in minimal form (if you need that, use ROBDD).
- The recursive approach means that the same cofactors are computed several times, so memoization should be implemented.
- Generally, tautology-checking, satisfiability (SAT) and other related problems are NP-complete (exponential time), so for large problems, the best-practice data structure should be used instead.
