#include <iostream>
#include <chrono>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

#include "Helpers/Src_Prjt-ES22BTECH11001-generateGraph.cpp"
#include "Helpers/Src_Prjt-ES22BTECH11001-checkColor.cpp"

#include "Baselines/Src_Prjt-ES22BTECH11001-greedyColoring.cpp"
#include "Baselines/Src_Prjt-ES22BTECH11001-welshPowelColoring.cpp"
#include "Baselines/Src_Prjt-ES22BTECH11001-DSATUR.cpp"
#include "Baselines/Src_Prjt-ES22BTECH11001-mColoring.cpp"

#include "GM_VARIANTS/Src_Prjt-ES22BTECH11001-GM_greedy.cpp"
#include "GM_VARIANTS/Src_Prjt-ES22BTECH11001-GM_prefetch.cpp"
#include "GM_VARIANTS/Src_Prjt-ES22BTECH11001-GM_lockfree_Phase2.cpp"
#include "GM_VARIANTS/Src_Prjt-ES22BTECH11001-GM_optimized_mex.cpp"
#include "GM_VARIANTS/Src_Prjt-ES22BTECH11001-GM_balance_edges.cpp"

#include "IterativeCSR/Src_Prjt-ES22BTECH11001-GM_iterative.cpp"
#include "IterativeCSR/Src_Prjt-ES22BTECH11001-GM_iterative_CSR.cpp"
#include "IterativeCSR/Src_Prjt-ES22BTECH11001-GM_sequential.cpp"

#include "Novel/Src_Prjt-ES22BTECH11001-GM_random_Phase1.cpp"
// #include "Novel/Src_Prjt-ES22BTECH11001-edgeLock_Naive.cpp"
#include "Novel/Src_Prjt-ES22BTECH11001-edgeLock_Portable.cpp"
// #include "Novel/Src_Prjt-ES22BTECH11001-edgeLock_CSR.cpp"

struct AlgorithmResult {
    int colorCount;
    vector<int> colors;
    bool hasColorOutput;
};

static void printSeparator() {
    for (int i = 0; i < 30; i++) cout << "*";
    cout << "\n";
}

template<typename RunFn>
void runAlgorithm(const string& title, RunFn&& run, CheckColoring& checker, const vector<vector<int>>& adj) {
    printSeparator();
    cout << "RUNNING " << title << "\n";

    auto start = high_resolution_clock::now();
    AlgorithmResult result = run();
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << result.colorCount << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    if (result.hasColorOutput) {
        cout << "Valid Coloring: " << checker.isValid(adj, result.colors) << endl;
    } else {
        cout << "Valid Coloring: skipped (no color output)" << endl;
    }
}

int main() {
    ifstream inFile("inp-params.txt");
    int n;
    double prob;
    int p;
    inFile >> n >> prob >> p;

    generateGraph g(n, prob);
    vector<vector<int>> adj = g.getGraph();
    CheckColoring checker;

    cout << "Graph Generated" << endl;

    struct AlgorithmJob {
        string title;
        function<AlgorithmResult()> run;
    };

    vector<AlgorithmJob> jobs = {
        // {"THE GREEDY COLORING ALGORITHM", [&]() {
        //     greedyColoring alg(adj);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE WELSH POWEL COLORING ALGORITHM", [&]() {
        //     welshPowelColoring alg(adj);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM (SPECTRAL) COLORING ALGORITHM", [&]() {
        //     GM alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM PREFETCH VARIANT", [&]() {
        //     GM_prefetch alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM LOCKFREE PHASE2 VARIANT", [&]() {
        //     GM_lockfree_Phase2 alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM OPTIMIZED MEX VARIANT", [&]() {
        //     GM_optimized_mex alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM BALANCE EDGES VARIANT", [&]() {
        //     GM_balance_edges alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM ITERATIVE ALGORITHM", [&]() {
        //     GM_iterative alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.get_color(), true};
        // }},
        // {"THE GM ITERATIVE CSR ALGORITHM", [&]() {
            // GM_iterative_CSR alg(adj, p);
            // return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM SEQUENTIAL CSR ALGORITHM", [&]() {
        //     GM_sequential alg(adj);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE GM RANDOM PHASE1 ALGORITHM", [&]() {
        //     GM_random_Phase1 alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        // {"THE EDGELOCK NAIVE ALGORITHM", [&]() {
        //     edgeLock_Naive alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }},
        {"THE EDGELOCK PORTABLE ALGORITHM", [&]() {
            edgeLock_Portable alg(adj, p);
            return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        }},
        // {"THE EDGELOCK CSR ALGORITHM", [&]() {
        //     edgeLock_CSR alg(adj, p);
        //     return AlgorithmResult{alg.getNoOfColors(), alg.return_color(), true};
        // }}
    };

    for (const auto& job : jobs) {
        runAlgorithm(job.title, job.run, checker, adj);
    }

    return 0;
}
