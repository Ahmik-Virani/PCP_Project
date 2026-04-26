/*
======================================================================================
FILE: GM_iterative.cpp
ALGORITHM: Iterative Lock-Free Gebremedhin-Manne
SYSTEMS LEVEL: 2 (Killing Sequential Bottlenecks)

OPTIMIZATIONS:
1. Iterative Worklists: Completely deleted the single-threaded Phase 3. Clashing nodes
   are packaged into a new 'worklist' and fed right back into the 16-core parallel engine.
2. Dynamic Thread Scaling: If the worklist gets too small (e.g., only 5 conflicts left), 
   it scales down to 1 thread to avoid the massive OS overhead of spinning up 16 threads 
   for tiny workloads.

REMAINING BOTTLENECK:
- Memory Wall. `vector<vector<int>>` fragments the heap, causing L1 Cache Misses.
======================================================================================
*/

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <numeric>

using namespace std;

class GM_iterative {
private:
    int n;
    vector<vector<int>> adj;
    int p;
    int noOfColors;

    // SYSTEMS NOTE: Instead of passing start/end indexes for the whole graph, 
    // we now pass indexes for a specific `worklist`. This allows us to re-run 
    // parallel phases on shrinking subsets of nodes.
    void pseudo_color_iter(int start_idx, int end_idx, const vector<int>& worklist, vector<int>& color) {
        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            vector<bool> color_nbr(n, false); 
            
            for (int nbr : adj[u]) {
                if (color[nbr] != -1) {
                    color_nbr[color[nbr]] = true;
                }
            }

            for (int j = 0; j < n; j++) {
                if (!color_nbr[j]) {
                    color[u] = j;
                    break;
                }
            }
        }
    }

    void check_clash_iter(int start_idx, int end_idx, const vector<int>& worklist, const vector<int>& color, vector<int>& local_clashes) {
        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            for (int nbr : adj[u]) {
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
        vector<int> color(n, -1);
        
        // SYSTEMS NOTE: The initial worklist contains all vertices.
        vector<int> worklist(n);
        iota(worklist.begin(), worklist.end(), 0); 

        // SYSTEMS NOTE: The Amdahl's Law killer. We keep looping in parallel 
        // until the worklist is totally empty. No single-threaded fallbacks.
        while (!worklist.size() == 0) {
            int current_work_size = worklist.size();
            
            // SYSTEMS NOTE: OS Thread Creation overhead is huge. 
            // If we only have 20 conflicts to fix, creating 16 threads will take longer 
            // than fixing them sequentially. This dynamically scales active threads.
            int active_threads = (current_work_size < p * 10) ? 1 : p;
            int chunk_size = current_work_size / active_threads;
            int extra = current_work_size % active_threads;

            vector<thread> threads(active_threads);
            int cur_ind = 0;
            for (int i = 0; i < active_threads; i++) {
                int start_idx = cur_ind;
                int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
                if (extra > 0) extra--;
                
                threads[i] = thread(&GM_iterative::pseudo_color_iter, this, start_idx, end_idx, cref(worklist), ref(color));
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
                
                threads[i] = thread(&GM_iterative::check_clash_iter, this, start_idx, end_idx, cref(worklist), cref(color), ref(thread_clashes[i]));
                cur_ind = end_idx + 1;
            }
            for (int i = 0; i < active_threads; i++) threads[i].join();

            // Refill the worklist for the next Iterative pass
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
    GM_iterative(vector<vector<int>> adj, int p) {
        n = adj.size();
        this->adj = adj;
        this->p = p;
        run_GM();
    }
    int getNoOfColors() { return noOfColors; }
};