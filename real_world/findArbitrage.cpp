#include<iostream>

#include "getRealWorldData.cpp"

using namespace std;

class findArbitrage {
private:
    vector<vector<pair<int, pair<double, double>>>> adj;
    vector<int> color;

public: 
    // get a graph at a particular time
    // also get the graph coloring
    findArbitrage(vector<vector<pair<int, pair<double,double>>>> adj, vector<int> color){
        this->adj = adj;
        this->color = color;
    }

};