#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <iomanip>

using namespace std;
using namespace std::chrono;

#include "generateGraphOpt.cpp"
#include "./GM_VARIANTS/GM_sequential.cpp"
#include "./GM_VARIANTS/GM_greedy.cpp"
#include "./GM_VARIANTS/GM_greedy_lockfree.cpp"
#include "./GM_VARIANTS/GM_iterative.cpp"
#include "./GM_VARIANTS/GM_iterative_csr.cpp"
#include "./JP_VARIANTS/JP_sequential.cpp"
#include "./JP_VARIANTS/JP_classic.cpp"
#include "./JP_VARIANTS/JP_optimized_csr.cpp"
#include "./JP_VARIANTS/JP_persistent_csr.cpp"
#include "greedyColoring.cpp"
#include "welshPowelColoring.cpp"
#include "mColoring.cpp"
#include "DSATUR.cpp"

int main() {
    int n;
    double prob;
    int p;

    // 1. Read parameters from input file
    ifstream infile("params.txt");
    if (!infile) {
        cerr << "Error: Could not open params.txt. Please create it with 'n prob p' format.\n";
        return 1;
    }
    infile >> n >> prob >> p;
    infile.close();

    // 2. Generate Graph
    // Uncomment this line to see generation time in terminal, but keep it out of the CSV parser
    // cerr << "Generating graph with n=" << n << ", prob=" << prob << ", threads=" << p << "...\n";
    generateGraph g(n, prob);
    vector<vector<int>> adj = g.getGraph();

    // 3. Print CSV Header (Python will read this)
    cout << "Algorithm,Colors,Time_us\n";

    // Macro for clean, repeatable timing and CSV output
    #define RUN_ALGO(NAME, CLASS_INIT) \
        { \
            auto start = steady_clock::now(); \
            CLASS_INIT; \
            auto stop = steady_clock::now(); \
            cout << NAME << "," << algo.getNoOfColors() << "," \
                 << duration_cast<microseconds>(stop - start).count() << "\n"; \
        }

    // --- SEQUENTIAL BASELINES ---
    RUN_ALGO("Greedy", greedyColoring algo(adj))
    RUN_ALGO("Welsh-Powell", welshPowelColoring algo(adj))
    RUN_ALGO("PureGreedy_CSR", PureGreedy_csr algo(adj))
    RUN_ALGO("JP_Sequential", JP_sequential_csr algo(adj))

    // --- GM VARIANTS ---
    RUN_ALGO("GM_Classic", GM algo(adj, p))
    RUN_ALGO("GM_LockFree", GM_lockfree algo(adj, p))
    RUN_ALGO("GM_Iterative", GM_iterative algo(adj, p))
    RUN_ALGO("GM_Iterative_CSR", GM_iterative_csr algo(adj, p))

    // --- JP VARIANTS ---
    RUN_ALGO("JP_Classic", JP_classic algo(adj, p))
    RUN_ALGO("JP_Optimized_CSR", JP_optimized_csr algo(adj, p))
    RUN_ALGO("JP_Persistent_CSR", JP_persistent_csr algo(adj, p))

    return 0;
}