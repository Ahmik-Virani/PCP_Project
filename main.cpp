#include <iostream>
#include <chrono>
#include <fstream>

using namespace std;
using namespace chrono;

#include "generateGraph.cpp"
#include "./GM_VARIANTS/GM_greedy.cpp"
#include "./GM_VARIANTS/GM_greedy_fetchIntoCache.cpp"
#include "./GM_VARIANTS/GM_greedy_lockfreePhase2.cpp"
#include "./GM_VARIANTS/GM_greedy_mex_wrong.cpp"
#include "./GM_VARIANTS/GM_greedy_partitionEdge.cpp"
#include "./Baselines/greedyColoring.cpp"
#include "./Baselines/welshPowelColoring.cpp"
#include "./Baselines/mColoring.cpp"
#include "./Baselines/DSATUR.cpp"
#include "./AV_Algo/allColor.cpp"
#include "./AV_Algo/allColorOpt.cpp"
#include "./AV_Algo/randColorAll.cpp"
#include "./AV_Algo/allColor_general.cpp"
#include "./AV_Algo/allColorSelfFirst.cpp"
#include "checkColor.cpp"
#include "./DD_Algo/GM_iterative_csr.cpp"
#include "./DD_Algo/GM_sequential.cpp"
#include "./AV_Algo/av_csr.cpp"
#include "./AV_Algo/allColorFinal.cpp"
#include "./DD_Algo/GM_iterative.cpp"

int main(){
    // specify the number of vertices and probability of edge
    ifstream inFile("inp-params.txt");

    int n;
    double prob;
    int p;

    inFile >> n >> prob >> p;

    // generate a graph with 'n' vertices and probability of edge 'prob'
    generateGraph g(n, prob);
    vector<vector<int>> adj = g.getGraph();

    CheckColoring checker;

    cout << "Graph Generated" << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GREEDY COLORING ALGORITHM\n";
    auto start = high_resolution_clock::now();
    greedyColoring greedy_approach(adj);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << greedy_approach.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, greedy_approach.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE WELSH POWEL COLORING ALGORITHM\n";
    start = high_resolution_clock::now();
    welshPowelColoring welsh_powel_approach(adj);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << welsh_powel_approach.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, welsh_powel_approach.return_color()) << endl;

    // for(int i = 0 ; i < 30 ; i++) cout << "*";
    // cout << "\nRUNNING THE BACKTRACKING (BRUTE FORCE) COLORING ALGORITHM\n";
    // start = high_resolution_clock::now();
    // minColor brute_force_approach(adj, welsh_powel_approach.getNoOfColors());
    // end = high_resolution_clock::now();
    // duration = duration_cast<microseconds>(end - start);
    // cout << "The number of colors: " << brute_force_approach.getNoOfColors() << "\n\n";
    // cout << "Time taken: " << duration.count() << " microseconds" << endl;

    // for(int i = 0 ; i < 30 ; i++) cout << "*";
    // cout << "\nRUNNING THE DSATUR COLORING ALGORITHM\n";
    // start = high_resolution_clock::now();
    // DSATUR DSATUR_approach(adj, welsh_powel_approach.getNoOfColors());
    // end = high_resolution_clock::now();
    // duration = duration_cast<microseconds>(end - start);
    // cout << "The number of colors: " << DSATUR_approach.getNoOfColors() << "\n\n";
    // cout << "Time taken: " << duration.count() << " microseconds" << endl;

    // GM Variants
    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM (SPECTRAL) COLORING ALGORITHM\n";
    start = high_resolution_clock::now();
    GM GM_parallel_approach(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << GM_parallel_approach.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, GM_parallel_approach.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM Optimization 1\n";
    start = high_resolution_clock::now();
    GM_opt1 GM_1(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << GM_1.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, GM_1.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM Optimization 2\n";
    start = high_resolution_clock::now();
    GM_opt2 GM_2(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << GM_2.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, GM_2.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM Optimization 3\n";
    start = high_resolution_clock::now();
    GM_opt3 GM_3(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << GM_3.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, GM_3.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM Optimization 4\n";
    start = high_resolution_clock::now();
    GM_opt4 GM_4(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << GM_4.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, GM_4.return_color()) << endl;

    // AV Variants
    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE AV all color\n";
    start = high_resolution_clock::now();
    GM_EdgeLock AV_1(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << AV_1.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, AV_1.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE AV all color Opt\n";
    start = high_resolution_clock::now();
    GM_EdgeLockOpt AV_2(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << AV_2.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, AV_2.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE AV rand color all\n";
    start = high_resolution_clock::now();
    randColorAll AV_3(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << AV_3.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " AV_3" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, greedy_approach.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE AV all color general\n";
    start = high_resolution_clock::now();
    GM_EdgeLock_General AV_4(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << AV_4.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, AV_4.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE AV all color self first\n";
    start = high_resolution_clock::now();
    GM_EdgeLock_Self AV_5(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << AV_5.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, AV_5.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE DD csr\n";
    start = high_resolution_clock::now();
    GM_iterative_csr DD_1(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << DD_1.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, DD_1.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE DD csr greedy\n";
    start = high_resolution_clock::now();
    PureGreedy_csr DD_2(adj);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << DD_2.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, DD_2.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE DD csr edge lock\n";
    start = high_resolution_clock::now();
    GM_EdgeLock_CSR AV_6(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << AV_6.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, AV_6.return_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE AV all color final\n";  // or any distinct name
    start = high_resolution_clock::now();
    GM_EdgeLock_Final AV_7(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << AV_7.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, AV_7.get_color()) << endl;

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM Iterative \n";  // or any distinct name
    start = high_resolution_clock::now();
    GM_iterative DD_3(adj, p);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "The number of colors: " << DD_3.getNoOfColors() << "\n\n";
    cout << "Time taken: " << duration.count() << " microseconds" << endl;
    cout << "Valid Coloring: " << checker.isValid(adj, DD_3.get_color()) << endl;

    return 0;
}