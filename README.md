# Parallel & Concurrent Programming - Project

This Project is made by : Ahmik Virani and Devraj
Roll Number : ES22BTECH11001, ES22BTECH11011

- This repository implements and compares multiple graph coloring algorithms.
- It includes baseline heuristics, GM variants, iterative CSR algorithms, and novel edge-lock implementations.
- The project is designed to evaluate runtime, number of colors, and correctness for randomly generated graphs.

## Input File
The input file is named `inp-params.txt` and should be placed in the repository root alongside `Src_Prjt-ES22BTECH11001-main.cpp`.

The file contains three values:
1. `n` — number of vertices
2. `edge_probability` — probability for each edge
3. `p` — algorithm-specific thread/partition parameter

Example content:

```text
50000 0.1 8
```

## Deployment
Build the project from the repository root using the actual source file name.

### Linux
```bash
g++ -O3 -std=c++20 -march=native -Wall -pthread -o gm_engine Src_Prjt-ES22BTECH11001-main.cpp
./gm_engine
```

### macOS
```bash
clang++ -O3 -std=c++20 -march=native -Wall -pthread -o gm_engine Src_Prjt-ES22BTECH11001-main.cpp
./gm_engine
```

### Windows (MinGW)
```bash
g++ -O3 -std=c++20 -Wall -pthread -o gm_engine.exe Src_Prjt-ES22BTECH11001-main.cpp
.\\gm_engine.exe
```

> If you use MSVC on Windows, convert the command to `cl /EHsc /std:c++20 Src_Prjt-ES22BTECH11001-main.cpp` and note that pthread support is not available by default.

## Output File
The program prints a summary for each algorithm to the console, including:
- number of colors used
- execution time in microseconds
- validity of the coloring

Additional log or output files are not created by default in this implementation.

## Project Structure
- `Src_Prjt-ES22BTECH11001-main.cpp` — entry point and runner for all algorithms
- `Helpers/`
    -`Src_Prjt-ES22BTECH11001-generateGraph.cpp` — random undirected graph generator
    - `Src_Prjt-ES22BTECH11001-checkColor.cpp` — validator for graph colorings
- `Baselines/`
  - `Src_Prjt-ES22BTECH11001-greedyColoring.cpp`
  - `Src_Prjt-ES22BTECH11001-welshPowelColoring.cpp`
  - `Src_Prjt-ES22BTECH11001-DSATUR.cpp`
  - `Src_Prjt-ES22BTECH11001-mColoring.cpp`
- `GM_VARIANTS/`
  - `Src_Prjt-ES22BTECH11001-GM_greedy.cpp`
  - `Src_Prjt-ES22BTECH11001-GM_prefetch.cpp`
  - `Src_Prjt-ES22BTECH11001-GM_lockfree_Phase2.cpp`
  - `Src_Prjt-ES22BTECH11001-GM_optimized_mex.cpp`
  - `Src_Prjt-ES22BTECH11001-GM_balance_edges.cpp`
- `IterativeCSR/`
  - `Src_Prjt-ES22BTECH11001-GM_iterative.cpp`
  - `Src_Prjt-ES22BTECH11001-GM_iterative_CSR.cpp`
  - `Src_Prjt-ES22BTECH11001-GM_sequential.cpp`
- `Novel/`
  - `Src_Prjt-ES22BTECH11001-GM_random_Phase1.cpp`
  - `Src_Prjt-ES22BTECH11001-edgeLock_Naive.cpp`
  - `Src_Prjt-ES22BTECH11001-edgeLock_Portable.cpp`
  - `Src_Prjt-ES22BTECH11001-edgeLock_CSR.cpp`

## Points to Keep in Mind
- The current `Src_Prjt-ES22BTECH11001-main.cpp` uses direct source inclusion for algorithm files.
- If you add a new algorithm implementation, include it in `Src_Prjt-ES22BTECH11001-main.cpp` and add it to the run list.
- Ensure `inp-params.txt` is present before running the program.
- The code is intended for experimental evaluation of parallel graph coloring approaches.

## Notes
- Some algorithms may be commented out in `Src_Prjt-ES22BTECH11001-main.cpp`; uncomment them to include them in a run.
- `p` is used by several algorithm variants as a thread or partition parameter.
- The graph generator produces random edges with the given edge probability.
