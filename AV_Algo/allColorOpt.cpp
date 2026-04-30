#include <os/lock.h> // The header for Apple-specific locks
#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>
#include <atomic>

using namespace std;

class GM_EdgeLockOpt {
private:
    // This is just a 32-bit integer under the hood, making it very cache-friendly
    os_unfair_lock edge_lock = OS_UNFAIR_LOCK_INIT;

    int n;  // The number of vertices
    vector<vector<int>> adj; // The adjacency Graph
    int p; // The number of processes/threads

    int noOfColors; // The number of colors used to color the graph

    vector<int> color;

    // a function which colors colors the graph
    // this is run in parallel by the threads
    void pseudo_color(int ind1, int ind2, vector<int> &color) {
        // Allocation ONCE per thread - scalable to any color count
        vector<bool> used_tracker(64, false);

        for (int i = ind1; i <= ind2; i++) {

            // if (i + 1 <= ind2) {
            //     __builtin_prefetch(&adj[i + 1], 0, 3);
            // }
            uint64_t fast_mask = 0;
            bool overflow = false;
            bool isCrossEdge = false;

            // --- Step 1: Collect Neighbor Colors ---
            for (int nbr : adj[i]) {
                int c = -1;
                if(nbr >= ind1 && nbr <= ind2){
                    // Internal neighbor: already colored if index < current
                    if (nbr < i) c = color[nbr];
                } else {
                    // [NOVELTY] Cross-neighbor: Lock to ensure we read a stable color
                    if (!isCrossEdge) {
                        os_unfair_lock_lock(&edge_lock);
                        isCrossEdge = true;
                    }
                    c = color[nbr];
                }

                if (c >= 0) {
                    if (c < 64) fast_mask |= (1ULL << c);
                    else overflow = true;
                }
            }

            // --- Step 2: Assign Color using Hybrid MEX ---
            if (!overflow && ~fast_mask != 0) {
                // Fast Path: Register-based math
                color[i] = __builtin_ctzll(~fast_mask);
            } else {
                // Slow Path: Scalable to any number of colors (1e6+)
                // Dynamically grow tracker if needed
                if (used_tracker.size() <= adj[i].size()) {
                    used_tracker.resize(adj[i].size() + 1, false);
                }

                // Mark all neighbors again in the tracker
                for (int nbr : adj[i]) {
                    int c = -1;
                    // Internal neighbor logic
                    if(nbr >= ind1 && nbr <= ind2){
                        if (nbr < i) c = color[nbr];
                    } else {
                        // Cross neighbor: Lock already held from Step 1
                        c = color[nbr];
                    }

                    if (c >= 0 && (size_t)c < used_tracker.size()) {
                        used_tracker[c] = true;
                    }
                }

                // Find the first hole
                int mex = 0;
                while (mex < (int)used_tracker.size() && used_tracker[mex]) {
                    mex++;
                }
                color[i] = mex;

                // SCRUB: Reset the bits we touched so the tracker is clean for node i+1
                for (int nbr : adj[i]) {
                    int c = -1;
                    if (nbr >= ind1) {
                        if (nbr < i) c = color[nbr];
                    } else {
                        c = color[nbr];
                    }
                    if (c >= 0 && (size_t)c < used_tracker.size()) {
                        used_tracker[c] = false;
                    }
                }
            }

            // --- Step 3: Release lock if it was acquired ---
            if (isCrossEdge) {
                os_unfair_lock_unlock(&edge_lock);
            }
        }
    }

    // This helper function finds the start/end vertex indices for each thread
    vector<pair<int, int>> get_edge_balanced_chunks() {
        vector<pair<int, int>> chunks;
        long long total_edges = 0;
        for (int i = 0; i < n; i++) total_edges += adj[i].size();

        long long target_per_thread = total_edges / p;
        int current_start = 0;
        long long current_sum = 0;

        for (int i = 0; i < n; i++) {
            current_sum += adj[i].size();
            // If this thread has reached its edge quota, or it's the last vertex
            if (current_sum >= target_per_thread && chunks.size() < (size_t)p - 1) {
                chunks.push_back({current_start, i});
                current_start = i + 1;
                current_sum = 0;
            }
        }
        // Final chunk for the remaining vertices
        chunks.push_back({current_start, n - 1});
        return chunks;
    }

    void run_GM(){
        // define a vector of colors
        color.assign(n, -1);

        vector<pair<int, int>> chunks = get_edge_balanced_chunks();

        // PHASE 1 - pseudo graph coloring

        // each thread will color a subset of edges from the graph
        // each thread will have approximatrly edges/p edges

        vector<thread> threads(p);
        for(int i = 0 ; i < p; i++){

            threads[i] = thread(&GM_EdgeLockOpt::pseudo_color, this, chunks[i].first, chunks[i].second, ref(color));

        }

        // wait for the threads to finish coloring
        for(int i = 0 ; i < p ; i ++){
            threads[i].join();
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
    GM_EdgeLockOpt (vector<vector<int>> adj, int p) {
        n = adj.size();
        this->adj = adj;
        this->p = p;

        run_GM();
    }

    int getNoOfColors(){
        return noOfColors;
    }

    vector<int> return_color(){
        return color;
    }
};