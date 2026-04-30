/*
======================================================================================
FILE: GM_iterative_csr.cpp
ALGORITHM: Iterative Lock-Free Gebremedhin-Manne with CSR
SYSTEMS LEVEL: 3 (Hardware Data Locality & Zero-Allocation Loops)

OPTIMIZATIONS:
1. CSR Layout (Compressed Sparse Row): Flattened `vector<vector<int>>` into two 1D 
   arrays (`row_ptr` and `col_ind`). The hardware prefetcher can perfectly predict this,
   virtually eliminating Cache Misses and RAM wait times.
2. Thread-Local Heap Re-use: Replaced `vector<bool>` inside the inner loop with a 
   `vector<int> forbidden_colors` allocated ONCE OUTSIDE the loop. This completely 
   eliminates heap allocations (`new`/`delete`) and `std::fill` during the hot path.

STATUS: 
- This is an enterprise-grade, high-performance CPU Graph Engine.
======================================================================================
*/

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <numeric>

using namespace std;

class GM_iterative_csr {
private:
    int n;
    int p;
    int noOfColors;

    // SYSTEMS NOTE: The CSR Data Structures. 
    // All edges in the entire graph sit directly next to each other in RAM.
    vector<int> row_ptr;
    vector<int> col_ind;

    vector<int> color;

    void pseudo_color_iter_csr(int start_idx, int end_idx, const vector<int>& worklist, vector<int>& color) {
        
        // SYSTEMS NOTE: This is allocated EXACTLY ONCE per thread on thread-boot.
        // It prevents the OS from constantly giving/taking heap memory.
        // We use an integer array where forbidden_colors[c] == u means color 'c' is forbidden for vertex 'u'.
        vector<int> forbidden_colors(n, -1);

        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            
            // SYSTEMS NOTE: CSR Traversal. We do two simple O(1) array lookups 
            // to find exactly where our neighbors begin and end in the massive 1D array.
            int edge_start = row_ptr[u];
            int edge_end = row_ptr[u + 1];

            // SYSTEMS NOTE: Because we iterate over a 1D array sequentially (`col_ind`),
            // the Intel/Apple L1 Cache prefetcher will automatically load the next 
            // cache line before the CPU even asks for it.
            for (int e = edge_start; e < edge_end; ++e) {
                int nbr = col_ind[e];
                int c = color[nbr];
                if (c != -1) {
                    // SYSTEMS NOTE: Instead of `array[c] = true` (which requires resetting 
                    // the whole array for the next vertex), we just stamp it with `u`.
                    // This creates a state machine that naturally resets itself!
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
    }

    void check_clash_iter_csr(int start_idx, int end_idx, const vector<int>& worklist, const vector<int>& color, vector<int>& local_clashes) {
        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            
            int edge_start = row_ptr[u];
            int edge_end = row_ptr[u + 1];

            for (int e = edge_start; e < edge_end; ++e) {
                int nbr = col_ind[e];
                if (color[u] == color[nbr]) {
                    if (u < nbr) {
                        local_clashes.push_back(u);
                        break; 
                    }
                }
            }
        }
    }

    void run_GM() {
        color.assign(n, -1);
        vector<int> worklist(n);
        iota(worklist.begin(), worklist.end(), 0); 

        while (!worklist.empty()) {
            int current_work_size = worklist.size();
            
            int active_threads = (current_work_size < p * 10) ? 1 : p;
            int chunk_size = current_work_size / active_threads;
            int extra = current_work_size % active_threads;

            vector<thread> threads(active_threads);
            int cur_ind = 0;
            for (int i = 0; i < active_threads; i++) {
                int start_idx = cur_ind;
                int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
                if (extra > 0) extra--;
                
                threads[i] = thread(&GM_iterative_csr::pseudo_color_iter_csr, this, start_idx, end_idx, cref(worklist), ref(color));
                cur_ind = end_idx + 1;
            }
            for (int i = 0; i < active_threads; i++) threads[i].join();

            vector<vector<int>> thread_clashes(active_threads);
            cur_ind = 0;
            extra = current_work_size % active_threads;
            for (int i = 0; i < active_threads; i++) {
                int start_idx = cur_ind;
                int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
                if (extra > 0) extra--;
                
                threads[i] = thread(&GM_iterative_csr::check_clash_iter_csr, this, start_idx, end_idx, cref(worklist), cref(color), ref(thread_clashes[i]));
                cur_ind = end_idx + 1;
            }
            for (int i = 0; i < active_threads; i++) threads[i].join();

            worklist.clear();
            for (int i = 0; i < active_threads; i++) {
                worklist.insert(worklist.end(), thread_clashes[i].begin(), thread_clashes[i].end());
            }
            
            for(int u : worklist) {
                color[u] = -1;
            }
        }

        noOfColors = 0;
        for (int i = 0; i < n; i++) { noOfColors = max(noOfColors, color[i]); }
        noOfColors++;
    }

public:
    // SYSTEMS NOTE: The constructor intercepts the slow `vector<vector<int>>` and 
    // instantly converts it to the fast CSR format before starting the parallel engine.
    GM_iterative_csr(const vector<vector<int>>& adj, int p) {
        n = adj.size();
        this->p = p;

        // --- CONVERT TO CSR FORMAT ---
        row_ptr.assign(n + 1, 0);
        
        // Count total edges to reserve memory ONCE, preventing vector reallocation overhead
        int total_edges = 0;
        for(int i = 0; i < n; i++) {
            total_edges += adj[i].size();
        }
        col_ind.reserve(total_edges);

        // Populate CSR arrays
        for(int i = 0; i < n; i++) {
            row_ptr[i] = col_ind.size();
            for(int nbr : adj[i]) {
                col_ind.push_back(nbr);
            }
        }
        row_ptr[n] = col_ind.size(); 
        
        run_GM();
    }

    int getNoOfColors() { return noOfColors; }

    vector<int> return_color() {
        return color;
    }
};