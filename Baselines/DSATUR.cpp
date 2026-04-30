#include<iostream>
#include<vector>
#include<set>

using namespace std;

class DSATUR {
private:
    int noOfColors;             // The number of colors used by this algorithm
    vector<vector<int>> adj;    // The adjacency graph
    int n;                      // The number of vertices

    // check if the graph can be colored using m colors
    // we use a recurssion
    bool check_m_coloring(const int &m, vector<int> &color){
        // base condition
        // check if all the vertices are colored
        bool done = true;
        for(int i = 0; i < n; i++){
            if(color[i] == -1){
                done = false;
                break;
            }
        }
        if(done) return true;

        int node = -1;
        int best_sat = -1;
        int best_deg = -1;

        for(int v = 0; v < n; v++){
            if(color[v] != -1) continue;

            // compute saturation of v
            set<int> s;
            for(int nbr : adj[v]){
                if(color[nbr] != -1){
                    s.insert(color[nbr]);
                }
            }

            int sat = s.size();

            if(sat > best_sat || (sat == best_sat && adj[v].size() > best_deg)){
                best_sat = sat;
                best_deg = adj[v].size();
                node = v;
            }
        }

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
            if(check_m_coloring(m, color)){
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
    DSATUR(vector<vector<int>> adj, int welsh_powell_result){
        n = adj.size();
        this->adj = adj;
        
        // we can check color in binary search manner
        // at max it will require the welsh_power_result number of vertices (this done since this is np hard)
        int l = 1, r = welsh_powell_result;
        while(l<r){
            // color array
            vector<int> color(n, -1);
            
            // The binary search loop
            int mid = l + (r-l)/2;
            if(check_m_coloring(mid, color)){
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