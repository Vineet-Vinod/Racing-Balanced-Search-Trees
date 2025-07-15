# Racing Balanced Search Trees

This project implements and compares the performance of different types of balanced search trees.

## Overview

This project provides implementations of the following balanced search trees:

- AVL Trees
- B-Trees
- Red-Black Trees (RB Trees)
- Splay Trees

The implementations are designed to be easily integrated into other projects. Each tree type is located in its own directory, containing the necessary header files.

## Usage

To use a specific tree implementation, include the corresponding `.h` file in your project. The tree classes are templated, allowing you to store various object types.

**Important:**  Objects stored in these trees must have:

- A `compare` function for comparison operations.
- Overloaded `==` and `!=` operators for equality checks.

## Benchmarking

To evaluate the performance of the different tree implementations:

1.  **Using Make:** If you have `make` installed, navigate to the main project directory in your terminal and run:

    ```bash
    make bench
    ```

    This will compile and run the benchmarking suite, providing performance results for each tree type.

2.  **Manual Compilation:** If `make` is not available, you can manually compile the project using your preferred C++ compiler.  Ensure that all necessary source files are included and then run the resulting executable.

## Project Structure

The project is organized into the following directories:

-   `AVL_Trees`: Contains the implementation of AVL trees.
-   `B_Trees`: Contains the implementation of B-Trees.
-   `RB_Trees`: Contains the implementation of Red-Black Trees.
-   `Splay_Trees`: Contains the implementation of Splay Trees.

Each directory will contain the header and source files specific to that tree implementation.
