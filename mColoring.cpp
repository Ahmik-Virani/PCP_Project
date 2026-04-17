#include<iostream>
#include<vector>

using namespace std;

class minColor {
private:
    int noOfColors;             // The number of colors used by this algorithm
    vector<vector<int>> adj;    // The adjacency graph
    int n;                      // The number of vertices

    // check if the graph can be colored using m colors
    // we use a recurssion
    bool check_m_coloring(const int &m, int index, vector<int> &color, const vector<pair<int, int>> &deg_index_adj){
        // base condition
        if(index == n) return true;

        int node = deg_index_adj[index].second;

        // check if any color from 0->m is possible
        for(int i = 0 ; i < m ; i++){
            // check if color i is possible
            bool isPossible = true;
            for(auto nbr : adj[node]){
                if(color[nbr] == i){
                    isPossible = false;
                    break;
                }
            }

            if(!isPossible) continue;

            color[node] = i;
            // then recursively call the other nodes
            if(check_m_coloring(m, index+1, color, deg_index_adj)){
                return true;
            }
            
            // if this did not work, redo the color to -1
            color[node] = -1;
        }
        
        // if at the end, no color works, return false
        return false;
    }
public:
    // The constructor
    // This takes in the graph, and finds the chromatic number
    minColor(vector<vector<int>> adj, int welsh_powell_result){
        n = adj.size();
        this->adj = adj;

        // sort the vertices
        vector<pair<int, int>> deg_index_adj(n);
        for(int i = 0 ; i < n ; i++){
            deg_index_adj[i] = {adj[i].size(), i};
        }
        sort(deg_index_adj.begin(), deg_index_adj.end(), greater<pair<int, int>>());

        
        // we can check color in binary search manner
        // at max it will require the welsh_power_result number of vertices (this done since this is np hard)
        int l = 1, r = welsh_powell_result;
        while(l<r){
            // color array
            vector<int> color(n, -1);
            
            // The binary search loop
            int mid = l + (r-l)/2;
            if(check_m_coloring(mid, 0, color, deg_index_adj)){
                r = mid;
            }else{
                l = mid+1;
            }

        }

        // r is the minimum number of colors we can color the graph using
        noOfColors = r;
    }

    int getNoOfColors(){
        return noOfColors;
    }
};