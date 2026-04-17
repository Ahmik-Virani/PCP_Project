#include<iostream>
#include<vector>
#include <random>

using namespace std;

class generateGraph{
private:
    int n;                      // number of vertices
    vector<vector<int>> adj;    // adjacency list
public:
    // constructor
    // it takes in number of vertices (n), and probability of an edge
    // and then generates a graph with n vertices
    generateGraph(int n, double p){
        this->n = n;

        // fix the size of the adjacency list
        adj.resize(n);

        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> dis(0.0, 1.0);

        // is there an edge between (i,j)
        for(int i = 0 ; i < n ; i++){
            for(int j = i+1 ; j < n ; j++){
                // with a probability p, there is an edge
                if(dis(gen) <= p){
                    adj[i].push_back(j);
                    adj[j].push_back(i);
                }
            }
        }
    }

    vector<vector<int>> getGraph(){
        return adj;
    }
};