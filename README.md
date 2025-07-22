# RayCompiler

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![Framework](https://img.shields.io/badge/Framework-LLVM-yellow.svg)](https://llvm.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/rayhuang2006/RayCompiler)

RayCompiler is a lightweight, educational Just-In-Time (JIT) compiler for a simple procedural language. Implemented in modern C++ and built upon the powerful LLVM compiler infrastructure, it serves as a practical demonstration of core compiler design principles, inspired by the official LLVM "Kaleidoscope" tutorial.

---

## Overview

This project implements a complete compiler pipeline, from source text to native machine code, demonstrating the fundamental stages of compilation:

1.  **Lexical Analysis**: The `Lexer` consumes raw source code and converts it into a stream of semantic tokens (e.g., `tok_def`, `tok_identifier`, `tok_number`).
2.  **Syntactic Analysis**: The `Parser` processes the token stream to build an Abstract Syntax Tree (AST). The AST is a hierarchical, in-memory representation of the source code's structure, defined in `include/ast.h`.
3.  **Code Generation**: The `Codegen` module traverses the AST and emits LLVM Intermediate Representation (IR). This IR is then passed to LLVM's JIT compilation engine, which dynamically compiles and executes the code at runtime.

This compiler is an ideal resource for anyone interested in learning how programming languages are implemented.

---

## Language Features

The custom language supported by RayCompiler is expression-oriented and includes:

-   **First-Class Functions**: Support for `def` to define functions with typed arguments.
-   **Conditional Logic**: `if/then/else` expressions for control flow.
-   **Binary Expressions**: Standard arithmetic (`+`, `-`, `*`, `/`) and comparison (`<`, `>`) operators with correct precedence parsing.
-   **Variable Bindings**: Simple variable assignment using the `=` operator within a function's scope.
-   **JIT Execution**: Code is compiled and run on-the-fly, with top-level expressions being wrapped in anonymous functions.
-   **Comments**: Ignores lines starting with `#`.

---

## Getting Started

Follow these instructions to build and run the compiler on your local machine.

### Prerequisites

-   A modern C++ toolchain (GCC or Clang). `clang++` is recommended.
-   The LLVM development libraries. Version 14 or newer is recommended.
-   `make` for simplified building.

On Debian/Ubuntu-based systems, you can install all prerequisites with:
```bash
sudo apt-get update
sudo apt-get install clang llvm-dev make
````

### Building the Compiler

A `Makefile` is provided to simplify the compilation process.

1.  Clone the repository:

    ```bash
    git clone git@github.com:rayhuang2006/RayCompiler.git
    cd RayCompiler
    ```

2.  Create a `Makefile` in the root directory with the following content:

    ```makefile
    # Makefile for RayCompiler

    # Compiler and flags
    CXX := clang++
    CXXFLAGS := -std=c++17 -Iinclude
    LLVM_FLAGS := $(shell llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native)

    # Source files and executable name
    SRCS := $(wildcard src/*.cpp)
    TARGET := ray_compiler

    .PHONY: all clean

    # Default target
    all: $(TARGET)

    # Linking the executable
    $(TARGET): $(SRCS)
    	$(CXX) $(CXXFLAGS) $(SRCS) $(LLVM_FLAGS) -o $(TARGET)
    	@echo "Build complete. Executable: $(TARGET)"

    # Clean up build artifacts
    clean:
    	rm -f $(TARGET)
    ```

3.  Run `make` to build the executable:

    ```bash
    make
    ```

    This will create the `ray_compiler` executable in the project root.

-----

## Usage

The compiler reads source from standard input and executes it.

1.  Create a source file, for example `example.ray`:

    ```cpp
    # example.ray - Demonstrates functions and conditionals.

    # A function to compute the hypotenuse.
    def hypot(a, b)
      (a*a + b*b); # The return value is the last expression

    # A function using a conditional.
    def check_sign(x)
      if x > 0 then
        1  # Positive
      else
        -1; # Negative or zero

    # Execute and print the result of top-level expressions.
    hypot(3, 4);  # Expected: 25 (since it returns a*a+b*b)
    check_sign(-5); # Expected: -1
    ```

2.  Pipe the source file into the compiler:

    ```bash
    ./ray_compiler < example.ray
    ```

The output will include the generated LLVM IR for each function followed by the computed results of the top-level expressions.

-----

## Project Structure

The repository is organized into a clean and logical structure to separate concerns.

```
RayCompiler/
├── .git/               # Git version control metadata
├── include/            # Header files for the core components
│   ├── ast.h           # Defines the Abstract Syntax Tree nodes
│   ├── lexer.h         # Public interface for the Lexer
│   └── parser.h        # Public interface for the Parser
├── src/                # Source code implementations
│   ├── codegen.cpp     # LLVM IR generation logic
│   ├── lexer.cpp       # Lexical analyzer implementation
│   └── parser.cpp      # Syntactic analyzer (parser) implementation
├── .gitignore          # Files and directories to be ignored by Git
└── README.md           # This file
```

-----

## Contributing

Contributions are welcome\! This project is an excellent platform for experimenting with new compiler features. If you have an idea for an improvement, please follow these steps:

1.  Fork the Project.
2.  Create your Feature Branch (`git checkout -b feature/AmazingFeature`).
3.  Commit your Changes (`git commit -m 'Add some AmazingFeature'`).
4.  Push to the Branch (`git push origin feature/AmazingFeature`).
5.  Open a Pull Request.

### Potential Future Work

  - [ ] Implement a `for` loop construct.
  - [ ] Add more data types (e.g., integers, booleans).
  - [ ] Enhance error reporting with source line and column numbers.
  - [ ] Add user-definable binary operators.
  - [ ] Build a small standard library of common functions.

-----

## License

Distributed under the MIT License.
