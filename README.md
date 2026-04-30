
# Parallel & Concurrent Programming

## Project Overview
This repository implements and compares several graph coloring algorithms, including classical heuristics and parallel / concurrent variants. The focus is on evaluating coloring quality, runtime, and correctness for randomly generated graphs.

## Authors
- Ahmik Virani, ES22BTECH11001
- Devraj, ES22BTECH11011

## Key Algorithms
- Greedy Coloring
- Welsh-Powell Coloring
- DSATUR (commented out in `main.cpp`)
- GM (spectral) algorithm variants
- AV algorithm variants
- DD algorithm variants
- Graph generation and color validation utilities

## Repository Structure
- `main.cpp` - project entry point and experiment runner
- `generateGraph.cpp` - random graph generator using `n` and edge probability
- `checkColor.cpp` - coloring validator
- `Baselines/` - baseline coloring algorithms
  - `greedyColoring.cpp`
  - `welshPowelColoring.cpp`
  - `mColoring.cpp`
  - `DSATUR.cpp`
- `GM_VARIANTS/` - GM algorithm variants and optimizations
- `AV_Algo/` - AV algorithm variants and optimizations
- `DD_Algo/` - DD algorithm variants and CSR implementations

## Input Parameters
The program reads a single input file named `inp-params.txt` with three values:

```text
<n> <edge_probability> <p>
```

Example:

```text
50000 0.1 8
```

Where:
- `n` = number of vertices in the generated graph
- `edge_probability` = probability of an edge between any two vertices
- `p` = algorithm-specific parameter used by GM / AV / DD variants

## Build and Run
From the repository root, compile and run the project with:

```bash
g++ -std=c++20 main.cpp -o pcp
./pcp
```

`main.cpp` includes the implementations of the selected algorithms by importing the source files directly.

## Output
The program prints results for each algorithm, including:
- number of colors used
- elapsed runtime in microseconds
- validity of the produced coloring

## Notes
- `main.cpp` currently runs multiple algorithms sequentially.
- Some algorithms in `main.cpp` are commented out and can be enabled for additional comparison.
- The graph generator produces an undirected random graph based on `inp-params.txt`.

## Extending the Project
To add or test a new algorithm:
1. Add the implementation file to an appropriate directory.
2. Include it in `main.cpp` and invoke it with the generated adjacency list.
3. Use `CheckColoring` to verify results.
