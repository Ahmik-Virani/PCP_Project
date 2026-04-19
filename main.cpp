#include <iostream>
#include <chrono> // Added for execution timing
using namespace std;
using namespace std::chrono; // Added for cleaner timing code

#include "generateGraphOpt.cpp"
#include "./GM_VARIANTS/GM_greedy.cpp"
#include "./GM_VARIANTS/GM_greedy_lockfree.cpp"
#include "./GM_VARIANTS/GM_iterative.cpp"
#include "./GM_VARIANTS/GM_iterative_csr.cpp"
#include "greedyColoring.cpp"
#include "welshPowelColoring.cpp"
#include "mColoring.cpp"
#include "DSATUR.cpp"

int main(){
    // specify the number of vertices and probability of edge
    int n = 1000000;
    double prob = 0.0001;
    int p = 8;

    // generate a graph with 'n' vertices and probability of edge 'prob'
    generateGraph g(n, prob);
    vector<vector<int>> adj = g.getGraph();

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GREEDY COLORING ALGORITHM\n";
    auto start = steady_clock::now();
    greedyColoring greedy_approach(adj);
    auto stop = steady_clock::now();
    cout << "The number of colors: " << greedy_approach.getNoOfColors() << "\n";
    cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE WELSH POWEL COLORING ALGORITHM\n";
    start = steady_clock::now();
    welshPowelColoring welsh_powel_approach(adj);
    stop = steady_clock::now();
    cout << "The number of colors: " << welsh_powel_approach.getNoOfColors() << "\n";
    cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    // for(int i = 0 ; i < 30 ; i++) cout << "*";
    // cout << "\nRUNNING THE BACKTRACKING (BRUTE FORCE) COLORING ALGORITHM\n";
    // start = steady_clock::now();
    // minColor brute_force_approach(adj, welsh_powel_approach.getNoOfColors());
    // stop = steady_clock::now();
    // cout << "The number of colors: " << brute_force_approach.getNoOfColors() << "\n";
    // cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    // for(int i = 0 ; i < 30 ; i++) cout << "*";
    // cout << "\nRUNNING THE DSATUR COLORING ALGORITHM\n";
    // start = steady_clock::now();
    // DSATUR DSATUR_approach(adj, welsh_powel_approach.getNoOfColors());
    // stop = steady_clock::now();
    // cout << "The number of colors: " << DSATUR_approach.getNoOfColors() << "\n";
    // cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM (SPECTRAL) COLORING ALGORITHM\n";
    start = steady_clock::now();
    GM GM_parallel_approach(adj, p);
    stop = steady_clock::now();
    cout << "The number of colors: " << GM_parallel_approach.getNoOfColors() << "\n";
    cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM LOCK-FREE COLORING ALGORITHM\n";
    start = steady_clock::now();
    GM_lockfree GM_lockfree_approach(adj, p); 
    stop = steady_clock::now();
    cout << "The number of colors: " << GM_lockfree_approach.getNoOfColors() << "\n";
    cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM ITERATIVE COLORING ALGORITHM\n";
    start = steady_clock::now();
    GM_iterative GM_iterative_approach(adj, p); 
    stop = steady_clock::now();
    cout << "The number of colors: " << GM_iterative_approach.getNoOfColors() << "\n";
    cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM ITERATIVE CSR ALGORITHM\n";
    start = steady_clock::now();
    GM_iterative_csr GM_csr_approach(adj, p);
    stop = steady_clock::now();
    cout << "The number of colors: " << GM_csr_approach.getNoOfColors() << "\n";
    cout << "Time taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n\n";

    return 0;
}