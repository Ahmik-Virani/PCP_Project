/*
======================================================================================
FILE: GM_greedy_lockfree.cpp
ALGORITHM: Lock-Free Gebremedhin-Manne Parallel Graph Coloring
SYSTEMS LEVEL: 1 (Thread-Local Storage & Lock-Free Logic)

OPTIMIZATIONS:
1. Lock-Free Filtering: Replaced `std::mutex` with a deterministic tie-breaker (`i < nbr`).
2. Thread-Local Storage (TLS): Replaced the global `std::set` with private `std::vector`s.
   Threads write exclusively to their own L1 cache lines, preventing "False Sharing".
3. Memory Contiguity: Replaced Red-Black tree (`std::set`) with flat Arrays (`std::vector`),
   drastically improving L1 cache hit rates.

REMAINING BOTTLENECK:
- Amdahl's Law is still active (Phase 3 is strictly sequential).
======================================================================================
*/

#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>

using namespace std;

class GM_lockfree {
private:    
    int n;  
    vector<vector<int>> adj; 
    int p; 
    int noOfColors; 

    void pseudo_color(int ind1, int ind2, vector<int> &color){
        for(int i = ind1 ; i <= ind2 ; i++){
            vector<bool> color_nbr(n, false);
            for(int nbr : adj[i]){
                if(nbr >= ind1 && nbr < i){
                    color_nbr[color[nbr]] = true;
                }
            }
            for(int j = 0 ; j < n ; j++){
                if(color_nbr[j] == false){
                    color[i] = j;
                    break;
                }
            }
        }
    }

    // SYSTEMS NOTE: By passing a private 'local_clashing' vector, this function 
    // now executes entirely in parallel with ZERO hardware contention.
    void check_clash_lockfree(int ind1, int ind2, const vector<int> &color, vector<int> &local_clashing) {
        for(int i = ind1 ; i <= ind2 ; i++){
            for(int nbr : adj[i]){
                if(color[i] == color[nbr]){
                    // SYSTEMS NOTE: The Lock-Free Tie-Breaker. 
                    // This mathematically guarantees that no two threads will ever 
                    // report the exact same clash. Duplicate filtering is achieved 
                    // without any slow data structures (like Sets) or Mutexes.
                    if(i < nbr){
                        local_clashing.push_back(i);
                        // SYSTEMS NOTE: Short-circuiting the inner loop saves thousands of 
                        // useless branch checks once the node is already flagged.
                        break; 
                    }
                }
            }
        }
    }

    void run_GM(){
        vector<int> color(n, -1);

        // --- PHASE 1 ---
        int cur_ind = 0;
        int extra_vertices = n%p;

        vector<thread> threads(p);
        for(int i = 0 ; i < p; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){ extra_vertices--; ind2++; }
            threads[i] = thread(&GM_lockfree::pseudo_color, this, ind1, ind2, ref(color));
            cur_ind = ind2 + 1;
        }

        for(int i = 0 ; i < p ; i ++){ threads[i].join(); }

        // --- PHASE 2 (LOCK-FREE) ---
        // SYSTEMS NOTE: Thread-Local Storage array. 
        // Thread 0 writes to index 0, Thread 1 writes to index 1.
        vector<vector<int>> thread_clashes(p);

        cur_ind = 0;
        extra_vertices = n % p;

        for(int i = 0 ; i < p ; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){ extra_vertices--; ind2++; }
            
            threads[i] = thread(&GM_lockfree::check_clash_lockfree, this, ind1, ind2, cref(color), ref(thread_clashes[i]));
            cur_ind = ind2 + 1;
        }

        for(int i = 0 ; i < p ; i++){ threads[i].join(); }

        // SYSTEMS NOTE: Flattening private vectors into one continuous block. 
        // Moving data sequentially like this is vastly faster than 16 threads 
        // fighting to write to one global array simultaneously.
        vector<int> final_clashing;
        for(int i = 0; i < p; i++){
            final_clashing.insert(final_clashing.end(), thread_clashes[i].begin(), thread_clashes[i].end());
        }

        // --- PHASE 3 ---
        for(int u : final_clashing){
            vector<int> nbr_colors;
            for(int nbr : adj[u]){
                nbr_colors.push_back(color[nbr]);
            }
            sort(nbr_colors.begin(), nbr_colors.end());
            int mex = 0;
            for(int i : nbr_colors){
                if(mex==i){ mex++; }
                else if(mex < i){ break; }
            }
            color[u] = mex;
        }

        noOfColors = 0;
        for(int i = 0 ; i < n ; i++){ noOfColors = max(noOfColors, color[i]); }
        noOfColors++;
    }

public:
    GM_lockfree(vector<vector<int>> adj, int p){
        n = adj.size();
        this->adj = adj;
        this->p = p;
        run_GM();
    }
    int getNoOfColors(){ return noOfColors; }
};