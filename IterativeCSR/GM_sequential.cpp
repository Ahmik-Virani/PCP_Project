#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class GM_sequential {
private:
    int n;
    int noOfColors;

    // --- CSR Data Structures ---
    vector<int> row_ptr;
    vector<int> col_ind;

    vector<int> color;

    void run_greedy() {
        color.assign(n,-1);
        
        // Allocate ONCE. No heap allocations in the hot path.
        vector<int> forbidden_colors(n, -1); 

        for (int u = 0; u < n; u++) {
            
            // O(1) lookup for the start and end of this vertex's neighbors
            int edge_start = row_ptr[u];
            int edge_end = row_ptr[u + 1];

            // Linear scan through contiguous memory (Cache Prefetcher's dream)
            for (int e = edge_start; e < edge_end; ++e) {
                int nbr = col_ind[e];
                int c = color[nbr];
                if (c != -1) {
                    forbidden_colors[c] = u; 
                }
            }

            for (int j = 0; j < n; j++) {
                if (forbidden_colors[j] != u) {
                    color[u] = j;
                    break;
                }
            }
        }

        noOfColors = 0;
        for (int i = 0; i < n; i++) {
            noOfColors = max(noOfColors, color[i]);
        }
        noOfColors++;
    }

public:
    GM_sequential(const vector<vector<int>>& adj) {
        this->n = adj.size();
        
        // --- INSTANT CSR CONVERSION ---
        row_ptr.assign(n + 1, 0);
        
        int total_edges = 0;
        for(int i = 0; i < n; i++) total_edges += adj[i].size();
        col_ind.reserve(total_edges);

        for(int i = 0; i < n; i++) {
            row_ptr[i] = col_ind.size();
            for(int nbr : adj[i]) col_ind.push_back(nbr);
        }
        row_ptr[n] = col_ind.size(); 
        
        run_greedy();
    }

    int getNoOfColors() { return noOfColors; }

    vector<int> return_color() {
        return color;
    }
};