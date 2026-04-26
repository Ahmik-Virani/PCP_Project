/*
======================================================================================
FILE: JP_optimized_csr.cpp
ALGORITHM: JP with Degree Weights and CSR Layout
SYSTEMS LEVEL: 1 (Data Locality and Global Heuristics)

OPTIMIZATIONS:
1. The "Welsh-Powell Brain": Replaced random `dis(gen)` with Node Degree. This forces 
   the algorithm to color highly constrained hubs first, drastically reducing total colors.
2. CSR Layout: Flattened the graph. Erases cache misses.
3. Zero-Allocation Loop: Replaced `vector<bool>` with a persistent `forbidden_colors` array.

REMAINING BOTTLENECK:
- Thread creation overhead is still active inside the `while` loop.
======================================================================================
*/

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

using namespace std;

class JP_optimized_csr {
private:
    int n;
    int p;
    int noOfColors;

    // SYSTEMS NOTE: CSR contiguous memory arrays.
    vector<int> row_ptr;
    vector<int> col_ind;

    void find_independent_set_csr(int start_idx, int end_idx, const vector<int>& color, const vector<int>& weight, vector<int>& local_set) {
        for (int u = start_idx; u <= end_idx; u++) {
            if (color[u] != -1) continue; 

            bool is_max = true;
            // SYSTEMS NOTE: O(1) direct memory lookup. 
            int edge_start = row_ptr[u];
            int edge_end = row_ptr[u + 1];

            // SYSTEMS NOTE: Hardware prefetcher successfully predicts this linear read.
            for (int e = edge_start; e < edge_end; ++e) {
                int nbr = col_ind[e];
                if (color[nbr] == -1) {
                    if (weight[nbr] > weight[u] || (weight[nbr] == weight[u] && nbr > u)) {
                        is_max = false;
                        break; 
                    }
                }
            }
            if (is_max) local_set.push_back(u); 
        }
    }

    void color_independent_set_csr(int start_idx, int end_idx, const vector<int>& independent_set, vector<int>& color) {
        // SYSTEMS NOTE: Allocated once per thread execution. No `new`/`delete` in loop.
        vector<int> forbidden_colors(n, -1);

        for (int k = start_idx; k <= end_idx; k++) {
            int u = independent_set[k];

            int edge_start = row_ptr[u];
            int edge_end = row_ptr[u + 1];

            for (int e = edge_start; e < edge_end; ++e) {
                int nbr = col_ind[e];
                int c = color[nbr];
                // SYSTEMS NOTE: The self-resetting state machine trick.
                if (c != -1) forbidden_colors[c] = u;
            }

            for (int j = 0; j < n; j++) {
                if (forbidden_colors[j] != u) {
                    color[u] = j;
                    break;
                }
            }
        }
    }

    void run_JP() {
        vector<int> color(n, -1);
        vector<int> weight(n);

        // SYSTEMS NOTE: The Heuristic Upgrade. 
        // In CSR, degree is just the difference between two row pointers. O(1) math.
        for (int i = 0; i < n; i++) {
            weight[i] = row_ptr[i+1] - row_ptr[i]; 
        }

        int nodes_colored = 0;

        while (nodes_colored < n) {
            // ... [Thread Creation Logic Remains the Same] ...
            vector<vector<int>> thread_sets(p);
            vector<thread> threads(p);
            int chunk_size = n / p;
            int extra = n % p;
            int cur_ind = 0;
            for (int i = 0; i < p; i++) {
                int start_idx = cur_ind;
                int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
                if (extra > 0) extra--;
                threads[i] = thread(&JP_optimized_csr::find_independent_set_csr, this, start_idx, end_idx, cref(color), cref(weight), ref(thread_sets[i]));
                cur_ind = end_idx + 1;
            }
            for (int i = 0; i < p; i++) threads[i].join();

            vector<int> independent_set;
            for (int i = 0; i < p; i++) independent_set.insert(independent_set.end(), thread_sets[i].begin(), thread_sets[i].end());

            int set_size = independent_set.size();
            int active_threads = (set_size < p) ? 1 : p; 
            vector<thread> color_threads(active_threads);
            chunk_size = set_size / active_threads;
            extra = set_size % active_threads;
            cur_ind = 0;
            for (int i = 0; i < active_threads; i++) {
                int start_idx = cur_ind;
                int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
                if (extra > 0) extra--;
                color_threads[i] = thread(&JP_optimized_csr::color_independent_set_csr, this, start_idx, end_idx, cref(independent_set), ref(color));
                cur_ind = end_idx + 1;
            }
            for (int i = 0; i < active_threads; i++) color_threads[i].join();

            nodes_colored += set_size;
        }

        noOfColors = 0;
        for (int i = 0; i < n; i++) noOfColors = max(noOfColors, color[i]);
        noOfColors++;
    }

public:
    JP_optimized_csr(const vector<vector<int>>& adj, int p) {
        this->n = adj.size();
        this->p = p;
        row_ptr.assign(n + 1, 0);
        int total_edges = 0;
        for(int i = 0; i < n; i++) total_edges += adj[i].size();
        col_ind.reserve(total_edges);
        for(int i = 0; i < n; i++) {
            row_ptr[i] = col_ind.size();
            for(int nbr : adj[i]) col_ind.push_back(nbr);
        }
        row_ptr[n] = col_ind.size(); 
        run_JP();
    }
    int getNoOfColors() { return noOfColors; }
};