# SFP driver unit tests

The test build uses CMake and fetches the pinned CppUTest 4.0 release on the
first configure. A C++ compiler, a C compiler, CMake 3.20 or newer, and Git are
required.

From the repository root, run:

```sh
cmake -S firmware/tests/sfp -B firmware/tests/sfp/build
cmake --build firmware/tests/sfp/build
ctest --test-dir firmware/tests/sfp/build --output-on-failure
```

The target compiles the production SFP driver against the small HAL test double
in `include/`; no STM32 hardware or firmware cross compiler is needed.
