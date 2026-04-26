/*
======================================================================================
FILE: JP_persistent_csr.cpp
ALGORITHM: JP with Persistent Threads and Zero-Copy Sets
SYSTEMS LEVEL: 2 (OS Scheduler Evasion & Custom Synchronization)

OPTIMIZATIONS:
1. Persistent Threads: Threads are created exactly ONCE. The iterative `while` loop 
   is moved *inside* the thread function.
2. Custom Barrier: Uses `std::condition_variable` to synchronize threads in user-space
   (mostly), rather than joining and re-creating kernel threads.
3. Zero-Copy Architecture: Threads no longer dump their local maxima into a global 
   vector. They find their own independent set and color it directly.
4. Lock-Free Atomics: Uses `std::atomic<int>` to globally track completion.

REMAINING BOTTLENECK:
- Kernel Sleep: `cv.wait()` still forces the CPU into ring-0 context switches. 
- Load Imbalance: If one thread gets a highly dense graph chunk, 15 threads sit idle.
======================================================================================
*/

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <condition_variable>

using namespace std;

// SYSTEMS NOTE: A custom sync primitive. When a thread finishes Phase 1, 
// it hits this barrier and "waits" for the other 15 threads to catch up.
class SimpleBarrier {
private:
    mutex m;
    condition_variable cv;
    int count;
    int limit;
    int generation;
public:
    SimpleBarrier(int count) : limit(count), count(count), generation(0) {}
    void wait() {
        unique_lock<mutex> lock(m);
        int gen = generation;
        if (--count == 0) {
            generation++;
            count = limit;
            cv.notify_all();
        } else {
            // SYSTEMS NOTE: The kernel-level context switch bottleneck.
            // The OS takes the core away from our program until notified.
            cv.wait(lock, [this, gen] { return gen != generation; });
        }
    }
};

class JP_persistent_csr {
private:
    int n;
    int p;
    int noOfColors;

    vector<int> row_ptr;
    vector<int> col_ind;
    
    // SYSTEMS NOTE: `std::atomic` prevents data races without using a heavy `std::mutex`.
    atomic<int> global_nodes_colored;

    void worker_thread(int thread_id, int start_idx, int end_idx, vector<int>& color, const vector<int>& weight, SimpleBarrier& barrier) {
        
        vector<int> local_independent_set;
        vector<int> forbidden_colors(n, -1); 

        // SYSTEMS NOTE: The loop is now INSIDE the thread. The thread never dies.
        // `memory_order_relaxed` tells the CPU not to flush the pipeline, 
        // because we don't care about strict ordering here, just the final value.
        while (global_nodes_colored.load(memory_order_relaxed) < n) {
            
            local_independent_set.clear();
            
            for (int u = start_idx; u <= end_idx; u++) {
                if (color[u] != -1) continue; 

                bool is_max = true;
                int edge_start = row_ptr[u];
                int edge_end = row_ptr[u + 1];

                for (int e = edge_start; e < edge_end; ++e) {
                    int nbr = col_ind[e];
                    if (color[nbr] == -1) {
                        if (weight[nbr] > weight[u] || (weight[nbr] == weight[u] && nbr > u)) {
                            is_max = false;
                            break; 
                        }
                    }
                }
                if (is_max) local_independent_set.push_back(u);
            }

            // SYSTEMS NOTE: Ensure all cores finished finding Maxima before anyone colors.
            barrier.wait();

            // SYSTEMS NOTE: Zero-copy execution. The thread colors its own set.
            for (int u : local_independent_set) {
                int edge_start = row_ptr[u];
                int edge_end = row_ptr[u + 1];

                for (int e = edge_start; e < edge_end; ++e) {
                    int nbr = col_ind[e];
                    int c = color[nbr];
                    if (c != -1) forbidden_colors[c] = u;
                }

                for (int j = 0; j < n; j++) {
                    if (forbidden_colors[j] != u) {
                        color[u] = j;
                        break;
                    }
                }
            }

            // SYSTEMS NOTE: Hardware-level atomic addition. Extremely fast.
            global_nodes_colored.fetch_add(local_independent_set.size(), memory_order_relaxed);

            // SYSTEMS NOTE: Prevent eager threads from rushing into the next wave.
            barrier.wait();
        }
    }

    void run_JP() {
        vector<int> color(n, -1);
        vector<int> weight(n);

        for (int i = 0; i < n; i++) weight[i] = row_ptr[i+1] - row_ptr[i]; 

        global_nodes_colored.store(0);
        SimpleBarrier sync_barrier(p);
        vector<thread> threads(p);

        int chunk_size = n / p;
        int extra = n % p;
        int cur_ind = 0;

        // SYSTEMS NOTE: Threads spawned exactly ONCE.
        for (int i = 0; i < p; i++) {
            int start_idx = cur_ind;
            int end_idx = cur_ind + chunk_size - 1 + (extra > 0 ? 1 : 0);
            if (extra > 0) extra--;

            threads[i] = thread(&JP_persistent_csr::worker_thread, this, i, start_idx, end_idx, ref(color), cref(weight), ref(sync_barrier));
            cur_ind = end_idx + 1;
        }

        for (int i = 0; i < p; i++) threads[i].join();

        noOfColors = 0;
        for (int i = 0; i < n; i++) noOfColors = max(noOfColors, color[i]);
        noOfColors++;
    }

public:
    JP_persistent_csr(const vector<vector<int>>& adj, int p) {
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