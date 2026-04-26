/*
======================================================================================
FILE: GM_greedy.cpp
ALGORITHM: Classic Gebremedhin-Manne (GM) Parallel Graph Coloring
SYSTEMS LEVEL: 0 (Baseline / Textbook Implementation)

OPTIMIZATIONS:
- None. This is the baseline parallel implementation. 

SYSTEMS BOTTLENECKS (Why this is slow):
1. The Mutex Bottleneck: `clashing_set_lock` forces 15 cores to sleep while 1 core writes.
2. The Heap Bottleneck: `std::set` allocates memory dynamically in a tree, destroying cache locality.
3. Amdahl's Law: Phase 3 is entirely sequential, meaning 15 cores sit completely idle.
4. Memory Fragmentation: `vector<vector<int>>` scatters memory across the RAM.
======================================================================================
*/

#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>

using namespace std;

class GM {
private:    
    // SYSTEMS NOTE: A hardware mutex forces a pipeline flush on the CPU. 
    // When multiple threads hit this, the OS must arbitrate, causing massive latency.
    mutex clashing_set_lock;

    int n;  
    // SYSTEMS NOTE: Array-of-pointers. High cache miss rate because neighbors 
    // are not guaranteed to be stored contiguously in RAM.
    vector<vector<int>> adj; 
    int p; 

    int noOfColors; 

    void pseudo_color(int ind1, int ind2, vector<int> &color){
        for(int i = ind1 ; i <= ind2 ; i++){
            // SYSTEMS NOTE: Allocating a vector inside a hot loop is very expensive.
            // This calls `new` and `delete` underneath, heavily taxing the memory allocator.
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

    void check_clash(int ind1, int ind2, const vector<int> &color, set<int> &clashing){
        for(int i = ind1 ; i <= ind2 ; i++){
            for(int nbr : adj[i]){
                if(color[i]==color[nbr]){
                    // SYSTEMS NOTE: The "Critical Section". If 16 threads find a clash 
                    // at the exact same microsecond, 15 of them are halted here.
                    clashing_set_lock.lock();
                    // SYSTEMS NOTE: std::set is a Red-Black tree. Inserting into it 
                    // requires chasing pointers across RAM and rebalancing the tree.
                    clashing.insert(min(i, nbr));
                    clashing_set_lock.unlock();
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
            
            // SYSTEMS NOTE: Asking the OS to create threads takes ~10-30 microseconds per thread.
            threads[i] = thread(&GM::pseudo_color, this, ind1, ind2, ref(color));
            cur_ind = ind2 + 1;
        }

        // SYSTEMS NOTE: Barrier synchronization. Fast threads idle while slow threads finish.
        for(int i = 0 ; i < p ; i ++){ threads[i].join(); }

        // --- PHASE 2 ---
        cur_ind = 0;
        extra_vertices = n%p;
        set<int> clashing;
        for(int i = 0 ; i < p ; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){ extra_vertices--; ind2++; }
            
            threads[i] = thread(&GM::check_clash, this, ind1, ind2, cref(color), ref(clashing));
            cur_ind = ind2 + 1;
        }

        for(int i = 0 ; i < p ; i++){ threads[i].join(); }

        // --- PHASE 3 ---
        // SYSTEMS NOTE: Amdahl's Law in action. 1 core works, (p-1) cores do nothing.
        // If clashing.size() is 50,000, this sequential block destroys all parallel gains.
        for(int u : clashing){
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
        for(int i = 0 ; i < n ; i++){
            noOfColors = max(noOfColors, color[i]);
        }
        noOfColors++;
    }

public:
    GM(vector<vector<int>> adj, int p){
        n = adj.size();
        this->adj = adj;
        this->p = p;
        run_GM();
    }
    int getNoOfColors(){ return noOfColors; }
};