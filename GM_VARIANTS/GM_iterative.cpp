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

    // Phase 1: Parallel Speculative Coloring on a specific WORKLIST of vertices
    void pseudo_color_iter(int start_idx, int end_idx, const vector<int>& worklist, vector<int>& color) {
        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            vector<bool> color_nbr(n, false); // Optimization note: this allocation is slow, but we keep it for simplicity for now
            
            for (int nbr : adj[u]) {
                if (color[nbr] != -1) {
                    color_nbr[color[nbr]] = true;
                }
            }

            // Find MEX
            for (int j = 0; j < n; j++) {
                if (!color_nbr[j]) {
                    color[u] = j;
                    break;
                }
            }
        }
    }

    // Phase 2: Parallel Conflict Checking on the WORKLIST
    void check_clash_iter(int start_idx, int end_idx, const vector<int>& worklist, const vector<int>& color, vector<int>& local_clashes) {
        for (int k = start_idx; k <= end_idx; k++) {
            int u = worklist[k];
            for (int nbr : adj[u]) {
                if (color[u] == color[nbr]) {
                    // Tie-breaker: only the smaller ID gets re-added to the worklist
                    if (u < nbr) {
                        local_clashes.push_back(u);
                        // Reset color so it gets re-colored next round
                        // (Safe to do here because 'u' is effectively locked to this thread)
                        break; 
                    }
                }
            }
        }
    }

    void run_GM() {
        vector<int> color(n, -1);
        
        // Initial worklist contains all vertices: 0, 1, 2, ..., n-1
        vector<int> worklist(n);
        iota(worklist.begin(), worklist.end(), 0); 

        // Loop until there are no more clashes
        while (!worklist.size() == 0) {
            int current_work_size = worklist.size();
            
            // If the worklist is tiny, don't bother spinning up 16 threads. Do it sequentially.
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
                
                threads[i] = thread(&GM_iterative::pseudo_color_iter, this, start_idx, end_idx, cref(worklist), ref(color));
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
                
                threads[i] = thread(&GM_iterative::check_clash_iter, this, start_idx, end_idx, cref(worklist), cref(color), ref(thread_clashes[i]));
                cur_ind = end_idx + 1;
            }
            for (int i = 0; i < active_threads; i++) threads[i].join();


            // --- FLATTEN CLASHES INTO NEW WORKLIST ---
            worklist.clear();
            for (int i = 0; i < active_threads; i++) {
                worklist.insert(worklist.end(), thread_clashes[i].begin(), thread_clashes[i].end());
            }
            
            // Reset colors for clashed vertices so they get correctly re-evaluated
            for(int u : worklist) {
                color[u] = -1;
            }
        }

        // Calculate total colors used
        noOfColors = 0;
        for (int i = 0; i < n; i++) {
            noOfColors = max(noOfColors, color[i]);
        }
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