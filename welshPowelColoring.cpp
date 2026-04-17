#include <iostream>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;

class welshPowelColoring {
private:
    int noOfColors;             // The number of colors used by this algorithm
    vector<vector<int>> adj;    // The adjacency graph
    int n;                      // The number of vertices

    void applyColoring(vector<pair<int, int>> deg_index_adj){
        // a set for all the colors used
        vector<int> color(n, -1);

        // Go through all the vertices, and apply the lowest color
        for(auto [_, i] : deg_index_adj){
            vector<bool> color_nbr(n, false);
            for(int nbr : adj[i]){
                // if neighbour is not colored, then skip
                if(color[nbr] == -1) continue;

                color_nbr[color[nbr]] = true;
            }

            // find the smallest available color
            for(int j = 0 ; j < n ; j++){
                if(color_nbr[j] == false){
                    color[i] = j;
                    break;
                }
            }
        }

        // we need to find the number of distinct colors used
        noOfColors = 0;
        for(int i = 0 ; i < n ; i++){
            noOfColors = max(noOfColors, color[i]);
        }

        // Since the color values are 0 indexes, the noOfColors is 1 + max value used in color array
        noOfColors++;
    }

public:
    // The constructor
    // This takes in the graph, and applies the Welsh Powel coloring algorithm
    welshPowelColoring(vector<vector<int>> adj){
        n = adj.size();
        this->adj = adj;

        // we first need to sort the graph in descending degree before applying greedy coloring

        vector<pair<int, int>> deg_index_adj(n);
        for(int i = 0 ; i < n ; i++){
            deg_index_adj[i] = {adj[i].size(), i};
        }
        sort(deg_index_adj.begin(), deg_index_adj.end(), greater<pair<int, int>>());

        // apply the graph coloring
        applyColoring(deg_index_adj);
    }

    int getNoOfColors(){
        return noOfColors;
    }
};