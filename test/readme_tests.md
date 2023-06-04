# Build targets
- To build and test the library, use the `bt.sh` script.
- To build the library and run the main file, use the `br.sh` script.

# Test suite for the Library
- GoogleTest is used for the entire collection of tests.
- Test suite / all test cases are the `.cpp` files in `test/Testfiles`.
- GoogleTest itself is in `test/GoogleTests_Framework`.

## Test Organization
- Test are organized into files related to areas of Library usage.
- The `test/CMakeLists.txt` defines which test files are compiled into the test executable.
- The test executable runs all included tests.

### Test identifiers
- Typical test setup per file:
    - One or more fixtures define(s) some test data, and
    - One or more tests use(s) the fixture(s) to make assertions on the behaviour of objects and functions.
- The test name IS the test description.
- Follows the format `TestFixture.TestName`.
- An example is `MatrixPromotion.MatrixOfIntCanBePromotedToDouble`.

## Ideas and tips for developing using TDD

### Three Rules of TDD:
    1. Write production code only to make a failing test pass.
    2. Write no more unit test than sufficient to fail. Compilation failures are failures.
    3. Write only the production code needed to pass the failing test.

### And when doing TDD...
 * Single-responsibility principle (SRP). A function should only "do" one thing, and it should only have one reason to change.
 * Incrementalism. Seek incremental feedback. Build bit by bit. Perhaps design interfaces first.
 * YAGNI: Ya ain't gonna need it... Don't build things until you need them.
 * Center tests around distinct behaviour, rather than around functions.
 * Continual experimentation, discovery, and refinement.
 * Use SOLID...
