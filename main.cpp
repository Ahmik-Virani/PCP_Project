#include <iostream>
using namespace std;

#include "generateGraph.cpp"
#include "./GM_VARIANTS/GM_greedy.cpp"
#include "greedyColoring.cpp"
#include "welshPowelColoring.cpp"
#include "mColoring.cpp"
#include "DSATUR.cpp"

int main(){
    // specify the number of vertices and probability of edge
    int n = 20;
    double prob = 0.5;
    int p=8;

    // generate a graph with 'n' vertices and probability of edge 'prob'
    generateGraph g(n, prob);
    vector<vector<int>> adj = g.getGraph();

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GREEDY COLORING ALGORITHM\n";
    greedyColoring greedy_approach(adj);
    cout << "The number of colors: " << greedy_approach.getNoOfColors() << "\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE WELSH POWEL COLORING ALGORITHM\n";
    welshPowelColoring welsh_powel_approach(adj);
    cout << "The number of colors: " << welsh_powel_approach.getNoOfColors() << "\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE BACKTRACKING (BRUTE FORCE) COLORING ALGORITHM\n";
    minColor brute_force_approach(adj, welsh_powel_approach.getNoOfColors());
    cout << "The number of colors: " << brute_force_approach.getNoOfColors() << "\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE DSATUR COLORING ALGORITHM\n";
    DSATUR DSATUR_approach(adj, welsh_powel_approach.getNoOfColors());
    cout << "The number of colors: " << DSATUR_approach.getNoOfColors() << "\n\n";

    for(int i = 0 ; i < 30 ; i++) cout << "*";
    cout << "\nRUNNING THE GM (SPECTRAL) COLORING ALGORITHM\n";
    GM GM_parallel_approach(adj, p);
    cout << "The number of colors: " << GM_parallel_approach.getNoOfColors() << "\n\n";

    return 0;
}