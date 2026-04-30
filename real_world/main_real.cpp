#include <iostream>
#include<vector>
#include<string>
#include<map>
#include<set>

using namespace std;

#include "getRealWorldData.cpp"
#include "./GM_VARIANTS/GM_greedy.cpp"
#include "greedyColoring.cpp"
#include "welshPowelColoring.cpp"
#include "mColoring.cpp"
#include "DSATUR.cpp"

int main(){
    // get the real world data
    getRealData realData;
    long long sz = realData.getSizeOfData();

    // declare the graph
    // not more than 50 currencies
    vector<vector<pair<int, pair<double, double>>>> adj(50);

    // lightweight edge copy
    vector<vector<int>> adj_edges;

    // a currency to index mapping
    map<string, int> currency_index;
    int idx = 0;

    // a set to check if the edge is present
    set<pair<string, string>> isPresent_edge;

    // go through each time stamp
    for(long long i = 0 ; i < sz ; i++){
        // get data for that time stamp
        Tick data = realData.getData(i);

        string currency_from = data.symbol_from;
        string currency_to = data.symbol_to;

        // define index if not there
        if(currency_index.find(currency_from) == currency_index.end()){
            currency_index[currency_from] = idx++;
        }

        if(currency_index.find(currency_to) == currency_index.end()){
            currency_index[currency_to] = idx++;
        }

        // if the edge is not threre
        if(isPresent_edge.find({currency_from, currency_to}) == isPresent_edge.end()){

            // add the edge
            adj[currency_index[currency_from]].push_back({currency_index[currency_to], {data.bid, data.ask}});
            adj_edges[currency_index[currency_from]].push_back(currency_index[currency_to]);

            // recolor the graph
            // give adj_edges to any graph coloring for testing

            // r un a bellman ford

            // add it to the set
            isPresent_edge.insert({currency_from, currency_to});
        }
        // if it is there then do an update
        else{
            // do a soft update
        }
    }

    return 0;
}