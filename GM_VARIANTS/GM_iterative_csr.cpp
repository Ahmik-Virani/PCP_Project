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

    // --- CSR Data Structures ---
    vector<int> row_ptr;
    vector<int> col_ind;

    // Phase 1: Parallel Speculative Coloring using CSR
    void pseudo_color_iter_csr(int start_idx, int end_idx, const vector<int>& worklist, vector<int>& color) {
        
        // OPTIMIZATION: Allocate the 'forbidden colors' tracker ONCE per thread, not per vertex!
        // We use an integer array where forbidden_colors[c] == u means color 'c' is forbidden for vertex 'u'.
        // This completely eliminates the need to run std::fill(false) or allocate memory in the loop.
        vector<int> forbidden_colors(n, -1);

        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            
            // CSR Traversal: Get the start and end of vertex u's neighbors
            int edge_start = row_ptr[u];
            int edge_end = row_ptr[u + 1];

            // Mark colors of already colored neighbors as forbidden FOR THIS VERTEX 'u'
            for (int e = edge_start; e < edge_end; ++e) {
                int nbr = col_ind[e];
                int c = color[nbr];
                if (c != -1) {
                    forbidden_colors[c] = u; 
                }
            }

            // Find MEX (Smallest available color)
            for (int j = 0; j < n; j++) {
                if (forbidden_colors[j] != u) {
                    color[u] = j;
                    break;
                }
            }
        }
    }

    // Phase 2: Parallel Conflict Checking using CSR
    void check_clash_iter_csr(int start_idx, int end_idx, const vector<int>& worklist, const vector<int>& color, vector<int>& local_clashes) {
        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            
            int edge_start = row_ptr[u];
            int edge_end = row_ptr[u + 1];

            for (int e = edge_start; e < edge_end; ++e) {
                int nbr = col_ind[e];
                if (color[u] == color[nbr]) {
                    // Tie-breaker: only the smaller ID gets re-added to the worklist
                    if (u < nbr) {
                        local_clashes.push_back(u);
                        break; 
                    }
                }
            }
        }
    }

    void run_GM() {
        vector<int> color(n, -1);
        
        vector<int> worklist(n);
        iota(worklist.begin(), worklist.end(), 0); 

        while (!worklist.empty()) {
            int current_work_size = worklist.size();
            
            int active_threads = (current_work_size < p * 10) ? 1 : p;
            int chunk_size = current_work_size / active_threads;
            int extra = current_work_size % active_threads;

            // --- LAUNCH PHASE 1 ---
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


            // --- LAUNCH PHASE 2 ---
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


            // --- FLATTEN CLASHES INTO NEW WORKLIST ---
            worklist.clear();
            for (int i = 0; i < active_threads; i++) {
                worklist.insert(worklist.end(), thread_clashes[i].begin(), thread_clashes[i].end());
            }
            
            for(int u : worklist) {
                color[u] = -1;
            }
        }

        noOfColors = 0;
        for (int i = 0; i < n; i++) {
            noOfColors = max(noOfColors, color[i]);
        }
        noOfColors++;
    }

public:
    // Constructor accepts the old vector<vector<int>> and instantly flattens it into CSR
    GM_iterative_csr(const vector<vector<int>>& adj, int p) {
        n = adj.size();
        this->p = p;

        // --- CONVERT TO CSR FORMAT ---
        row_ptr.assign(n + 1, 0);
        
        // First pass: count total edges to reserve memory exactly once
        int total_edges = 0;
        for(int i = 0; i < n; i++) {
            total_edges += adj[i].size();
        }
        col_ind.reserve(total_edges);

        // Second pass: populate CSR arrays
        for(int i = 0; i < n; i++) {
            row_ptr[i] = col_ind.size();
            for(int nbr : adj[i]) {
                col_ind.push_back(nbr);
            }
        }
        row_ptr[n] = col_ind.size(); 
        // -----------------------------

        run_GM();
    }

    int getNoOfColors() { return noOfColors; }
};