/*
======================================================================================
FILE: JP_classic.cpp
ALGORITHM: Classic Jones-Plassmann (JP) Parallel Graph Coloring
SYSTEMS LEVEL: 0 (Baseline / Textbook Implementation)

OPTIMIZATIONS:
- None. This is the mathematically pure, but hardware-naive implementation.

SYSTEMS BOTTLENECKS (Why this is slow on CPUs):
1. OS Thread Thrashing: The `while` loop creates and destroys threads for every 
   independent set wave. For 1,000,000 nodes, this means calling the OS to schedule 
   threads hundreds of times.
2. The "Blind" Heuristic: Assigning random weights means the algorithm ignores 
   graph structure (unlike Welsh-Powell), resulting in a high number of colors.
3. Memory Copy Overhead: 16 threads dump their private arrays into one massive 
   global `independent_set` array during every single wave, wasting memory bandwidth.
======================================================================================
*/

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <random>

using namespace std;

class JP_classic {
private:
    int n;
    // SYSTEMS NOTE: Array of pointers. Kills cache locality.
    vector<vector<int>> adj;
    int p;
    int noOfColors;

    void find_independent_set(int start_idx, int end_idx, const vector<int>& color, const vector<double>& weight, vector<int>& local_set) {
        for (int u = start_idx; u <= end_idx; u++) {
            if (color[u] != -1) continue; 

            bool is_max = true;
            for (int nbr : adj[u]) {
                if (color[nbr] == -1) {
                    // SYSTEMS NOTE: Branching logic here is unpredictable, causing 
                    // Pipeline Flushes in the CPU's branch predictor.
                    if (weight[nbr] > weight[u] || (weight[nbr] == weight[u] && nbr > u)) {
                        is_max = false;
                        break; 
                    }
                }
            }
            if (is_max) local_set.push_back(u); 
        }
    }

    void color_independent_set(int start_idx, int end_idx, const vector<int>& independent_set, vector<int>& color) {
        for (int k = start_idx; k <= end_idx; k++) {
            int u = independent_set[k];

            // SYSTEMS NOTE: Heap allocation inside a tight loop. Very slow.
            vector<bool> color_nbr(n, false);
            for (int nbr : adj[u]) {
                if (color[nbr] != -1) color_nbr[color[nbr]] = true;
            }

            for (int j = 0; j < n; j++) {
                if (!color_nbr[j]) {
                    color[u] = j;
                    break;
                }
            }
        }
    }

    void run_JP() {
        vector<int> color(n, -1);
        vector<double> weight(n);

        mt19937 gen(42); 
        uniform_real_distribution<double> dis(0.0, 1.0);
        // SYSTEMS NOTE: Random weights result in "Blind" coloring. High color counts.
        for (int i = 0; i < n; i++) weight[i] = dis(gen);

        int nodes_colored = 0;

        // SYSTEMS NOTE: The OS Killer. This loop will run ~50-100 times for dense graphs.
        while (nodes_colored < n) {
            
            vector<vector<int>> thread_sets(p);
            vector<thread> threads(p);

            int chunk_size = n / p;
            int extra = n % p;
            int cur_ind = 0;

            // SYSTEMS NOTE: Asking the OS to schedule p threads.
            for (int i = 0; i < p; i++) {
                int start_idx = cur_ind;
                int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
                if (extra > 0) extra--;

                threads[i] = thread(&JP_classic::find_independent_set, this, start_idx, end_idx, cref(color), cref(weight), ref(thread_sets[i]));
                cur_ind = end_idx + 1;
            }

            for (int i = 0; i < p; i++) threads[i].join();

            // SYSTEMS NOTE: Flattening overhead. The CPU is forced to copy memory 
            // from 16 different locations into a new continuous block.
            vector<int> independent_set;
            for (int i = 0; i < p; i++) {
                independent_set.insert(independent_set.end(), thread_sets[i].begin(), thread_sets[i].end());
            }

            int set_size = independent_set.size();
            
            // SYSTEMS NOTE: Dynamic scaling prevents spawning 16 threads for 5 nodes.
            int active_threads = (set_size < p) ? 1 : p; 
            vector<thread> color_threads(active_threads);

            chunk_size = set_size / active_threads;
            extra = set_size % active_threads;
            cur_ind = 0;

            for (int i = 0; i < active_threads; i++) {
                int start_idx = cur_ind;
                int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
                if (extra > 0) extra--;

                color_threads[i] = thread(&JP_classic::color_independent_set, this, start_idx, end_idx, cref(independent_set), ref(color));
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
    JP_classic(vector<vector<int>> adj, int p) {
        this->n = adj.size();
        this->adj = adj;
        this->p = p;
        run_JP();
    }
    int getNoOfColors() { return noOfColors; }
};