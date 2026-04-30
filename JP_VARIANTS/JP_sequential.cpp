/*
======================================================================================
FILE: JP_sequential_csr.cpp
ALGORITHM: Sequential Jones-Plassmann (JP) with CSR
SYSTEMS LEVEL: Single-Core Parallel Simulation

OPTIMIZATIONS:
1. CSR Layout (Compressed Sparse Row): Flattens the graph into contiguous memory.
2. The "Welsh-Powell Brain": Uses Node Degree as weights to minimize color count.
3. Zero-Allocation Loop: Uses the `forbidden_colors` array stamp trick.

SYSTEMS NOTE:
- This algorithm simulates the "Independent Set Waves" of JP, but on a single core. 
- It proves that parallel algorithms often perform *more total operations* than 
  pure sequential algorithms (like PureGreedy). It will scan uncolored vertices 
  multiple times across many loops, exposing algorithmic overhead.
======================================================================================
*/

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class JP_sequential_csr {
private:
    int n;
    int noOfColors;

    // --- CSR Data Structures ---
    vector<int> row_ptr;
    vector<int> col_ind;

    void run_JP_sequential() {
        vector<int> color(n, -1);
        vector<int> weight(n);

        // SYSTEMS NOTE: Heuristic Upgrade. 
        // Degree is just the difference between two row pointers.
        for (int i = 0; i < n; i++) {
            weight[i] = row_ptr[i+1] - row_ptr[i]; 
        }

        // Allocate ONCE. No heap allocations in the hot path.
        vector<int> forbidden_colors(n, -1); 
        vector<int> independent_set;
        // Pre-allocate to avoid dynamic resizing during the loops
        independent_set.reserve(n); 

        int nodes_colored = 0;

        // SYSTEMS NOTE: The Wave Loop. 
        // In a dense 1,000,000 node graph, this loop might run 50+ times.
        while (nodes_colored < n) {
            
            independent_set.clear();

            // --- PHASE 1: Find Local Maxima (Sequentially) ---
            for (int u = 0; u < n; u++) {
                if (color[u] != -1) continue; 

                bool is_max = true;
                int edge_start = row_ptr[u];
                int edge_end = row_ptr[u + 1];

                for (int e = edge_start; e < edge_end; ++e) {
                    int nbr = col_ind[e];
                    if (color[nbr] == -1) {
                        // Strict Tie-breaker
                        if (weight[nbr] > weight[u] || (weight[nbr] == weight[u] && nbr > u)) {
                            is_max = false;
                            break; 
                        }
                    }
                }
                
                if (is_max) {
                    independent_set.push_back(u); 
                }
            }

            // --- PHASE 2: Color the Independent Set (Sequentially) ---
            for (int u : independent_set) {
                int edge_start = row_ptr[u];
                int edge_end = row_ptr[u + 1];

                for (int e = edge_start; e < edge_end; ++e) {
                    int nbr = col_ind[e];
                    int c = color[nbr];
                    if (c != -1) {
                        forbidden_colors[c] = u; // O(1) state machine stamp
                    }
                }

                for (int j = 0; j < n; j++) {
                    if (forbidden_colors[j] != u) {
                        color[u] = j;
                        break;
                    }
                }
            }

            nodes_colored += independent_set.size();
        }

        noOfColors = 0;
        for (int i = 0; i < n; i++) {
            noOfColors = max(noOfColors, color[i]);
        }
        noOfColors++;
    }

public:
    JP_sequential_csr(const vector<vector<int>>& adj) {
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
        
        run_JP_sequential();
    }

    int getNoOfColors() { return noOfColors; }
};