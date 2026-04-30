#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>
#include <atomic>

using namespace std;

class GM_opt2 {
private:    
    int n;  // The number of vertices
    vector<vector<int>> adj; // The adjacency Graph
    int p; // The number of processes/threads

    int noOfColors; // The number of colors used to color the graph

    vector<int> color;

    // a function which colors colors the graph
    // this is run in parallel by the threads
    void pseudo_color(int ind1, int ind2, vector<int> &color){
        // Allocate ONCE per thread
        vector<bool> used_tracker(64, false);

        for(int i = ind1 ; i <= ind2 ; i++){
            uint64_t fast_mask = 0;
            bool overflow = false;

            // Step 1: Collect neighbor colors
            for(int nbr : adj[i]){
                // Only neighbors in this block that are already colored
                if(nbr >= ind1 && nbr < i){
                    int c = color[nbr];
                    if(c >= 0){
                        if(c < 64) fast_mask |= (1ULL << c);
                        else overflow = true;
                    }
                }
            }

            // Step 2: Assign Color
            if(!overflow && ~fast_mask != 0){
                color[i] = __builtin_ctzll(~fast_mask);
            } else {
                // Scalable Path
                if(used_tracker.size() <= adj[i].size()) used_tracker.resize(adj[i].size() + 1, false);
                
                for(int nbr : adj[i]){
                    if(nbr >= ind1 && nbr < i){
                        int c = color[nbr];
                        if(c >= 0 && (size_t)c < used_tracker.size()) used_tracker[c] = true;
                    }
                }

                int mex = 0;
                while(used_tracker[mex]) mex++;
                color[i] = mex;

                // Step 3: Scrub tracker
                for(int nbr : adj[i]){
                    if(nbr >= ind1 && nbr < i){
                        int c = color[nbr];
                        if(c >= 0 && (size_t)c < used_tracker.size()) used_tracker[c] = false;
                    }
                }
            }
        }
    }

    void check_clash(int ind1, int ind2, const vector<int> &color, vector<atomic<bool>> &is_clashing){

        for(int i = ind1 ; i <= ind2 ; i++){
            for(int nbr : adj[i]){
                if(color[i]==color[nbr]){
                    // We mark the vertex with the smaller index to be re-colored.
                    is_clashing[min(i, nbr)].store(true, memory_order_relaxed);
                }
            }
        }
    }

    void run_GM(){
        // define a vector of colors
        color.assign(n, -1);

        // PHASE 1 - pseudo graph coloring

        // each thread will color a subset of vertices from the graph
        // each thread will have approximatrly n/p vertices

        int cur_ind = 0;
        int extra_vertices = n%p;

        vector<thread> threads(p);
        for(int i = 0 ; i < p; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){
                extra_vertices--;
                ind2++;
            }
            threads[i] = thread(&GM_opt2::pseudo_color, this, ind1, ind2, ref(color));

            // update the current index
            cur_ind = ind2 + 1;
        }

        // wait for the threads to finish coloring
        for(int i = 0 ; i < p ; i ++){
            threads[i].join();
        }

        // PHASE 2 - check wrong colors
        
        // we have a set storing the vertices who have same colors adjacent to each other
        cur_ind = 0;
        extra_vertices = n%p;
        vector<atomic<bool>> is_clashing(n);
        for(int i=0; i<n; ++i) is_clashing[i].store(false);

        for(int i = 0 ; i < p ; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){
                extra_vertices--;
                ind2++;
            }
            threads[i] = thread(&GM_opt2::check_clash, this, ind1, ind2, cref(color), ref(is_clashing));

            // update the current index
            cur_ind = ind2 + 1;
        }

        // wait for the threads to finish their checks
        for(int i = 0 ; i < p ; i++){
            threads[i].join();
        }
        
        // PHASE 3 - color the vertices sequentially
        vector<bool> used_tracker(64, false);
        for(int i = 0; i < n; i++) {
            if (is_clashing[i].load(memory_order_relaxed)) {
                uint64_t fast_mask = 0;
                bool overflow = false;

                // Step 1: Attempt Fast-Path (First 64 colors)
                for (int nbr : adj[i]) {
                    int c = color[nbr];
                    if (c >= 0) {
                        if (c < 64) fast_mask |= (1ULL << c);
                        else overflow = true;
                    }
                }
                // If the first 64 colors aren't full and no high colors exist, use hardware scan
                if (!overflow && ~fast_mask != 0) {
                    color[i] = __builtin_ctzll(~fast_mask);
                }
                else {
                // Step 2: Slow-Path (Scalable to any degree)
                // Resize tracker if current node has potential for more colors than we've seen
                if (used_tracker.size() <= adj[i].size()) {
                    used_tracker.resize(adj[i].size() + 1, false);
                }

                // Mark used colors
                for (int nbr : adj[i]) {
                    int c = color[nbr];
                    if (c >= 0 && (size_t)c < used_tracker.size()) {
                        used_tracker[c] = true;
                    }
                }

                // Find the first hole
                int mex = 0;
                while (used_tracker[mex]) {
                    mex++;
                }
                color[i] = mex;

                // Step 3: "Scrub" the tracker (Resetting it for the next node)
                // Do NOT use used_tracker.assign(n, false) as that is O(N)
                // Scrubbing by iterating neighbors is O(Degree), which is much faster.
                for (int nbr : adj[i]) {
                    int c = color[nbr];
                    if (c >= 0 && (size_t)c < used_tracker.size()) {
                        used_tracker[c] = false;
                    }
                }
            }
        }
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

    // Constructor
    // give the graph and number of processes
    // p is 1 for sequential
    GM_opt2(vector<vector<int>> adj, int p){
        n = adj.size();
        this->adj = adj;
        this->p = p;

        run_GM();
    }

    int getNoOfColors(){
        return noOfColors;
    }

    vector<int> return_color() {
        return color;
    }

};