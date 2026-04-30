#include <os/lock.h>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

using namespace std;

class edgeLock_CSR {
private:
    os_unfair_lock edge_lock = OS_UNFAIR_LOCK_INIT;
    int n;
    int p;
    int noOfColors;

    // --- Contiguous CSR Storage ---
    // row_ptr[i] stores the starting index in col_ind for vertex i
    vector<int> row_ptr; 
    // col_ind stores all neighbor IDs back-to-back in one single memory block
    vector<int> col_ind; 
    
    vector<int> color;

    void pseudo_color(int ind1, int ind2) {
        // PER-THREAD tracker using the "Stamp" trick to avoid re-clearing memory
        // Initialized to -1.
        vector<int> forbidden_colors(64, -1); 

        for (int i = ind1; i <= ind2; i++) {
            int start = row_ptr[i];
            int end = row_ptr[i+1];
            
            // Ensure tracker is large enough for this node's degree
            if ((int)forbidden_colors.size() < (end - start + 1)) {
                forbidden_colors.resize((end - start) * 2, -1);
            }

            bool isCrossEdge = false;

            // Step 1 & 2: Single pass over contiguous neighbors
            for (int e = start; e < end; e++) {
                int nbr = col_ind[e];

                if (nbr >= ind1 && nbr <= ind2) {
                    // Local neighbor
                    if (nbr < i) {
                        int c = color[nbr];
                        if (c != -1) forbidden_colors[c] = i;
                    }
                } else {
                    // Cross neighbor - Grab lock
                    if (!isCrossEdge) {
                        os_unfair_lock_lock(&edge_lock);
                        isCrossEdge = true;
                    }
                    int c = color[nbr];
                    if (c != -1) {
                        if (c >= (int)forbidden_colors.size()) {
                            forbidden_colors.resize(c + 1, -1);
                        }
                        forbidden_colors[c] = i;
                    }
                }
            }

            // Step 3: Find MEX using the stamp 'i'
            int j = 0;
            while (j < (int)forbidden_colors.size() && forbidden_colors[j] == i) {
                j++;
            }
            color[i] = j;

            if (isCrossEdge) {
                os_unfair_lock_unlock(&edge_lock);
            }
        }
    }

    // CSR-based edge balancer (much faster with contiguous arrays)
    vector<pair<int, int>> get_edge_balanced_chunks() {
        vector<pair<int, int>> chunks;
        long long total_edges = col_ind.size();
        long long target_per_thread = total_edges / p;
        
        int current_start = 0;
        long long current_sum = 0;

        for (int i = 0; i < n; i++) {
            current_sum += (row_ptr[i+1] - row_ptr[i]);
            if (current_sum >= target_per_thread && chunks.size() < (size_t)p - 1) {
                chunks.push_back({current_start, i});
                current_start = i + 1;
                current_sum = 0;
            }
        }
        chunks.push_back({current_start, n - 1});
        return chunks;
    }

public:
    edgeLock_CSR(const vector<vector<int>>& adj, int p) : p(p) {
        this->n = adj.size();
        
        // --- Conversion to Contiguous CSR ---
        row_ptr.reserve(n + 1);
        long long total_edges = 0;
        for(const auto& v : adj) total_edges += v.size();
        col_ind.reserve(total_edges);

        for (int i = 0; i < n; i++) {
            row_ptr.push_back((int)col_ind.size());
            for (int nbr : adj[i]) {
                col_ind.push_back(nbr);
            }
        }
        row_ptr.push_back((int)col_ind.size());

        color.assign(n, -1);
        auto chunks = get_edge_balanced_chunks();

        vector<thread> threads;
        for (int i = 0; i < p; i++) {
            threads.emplace_back(&edgeLock_CSR::pseudo_color, this, chunks[i].first, chunks[i].second);
        }

        for (auto& t : threads) t.join();

        noOfColors = 0;
        for (int i = 0; i < n; i++) noOfColors = max(noOfColors, color[i]);
        noOfColors++;
    }

    int getNoOfColors() { return noOfColors; }
    vector<int> return_color() { return color; }
};