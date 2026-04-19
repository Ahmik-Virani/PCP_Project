#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>

using namespace std;

class GM_lockfree {
private:    
    // lock for the clashing set used in phase 2
    mutex clashing_set_lock;

    int n;  // The number of vertices
    vector<vector<int>> adj; // The adjacency Graph
    int p; // The number of processes/threads

    int noOfColors; // The number of colors used to color the graph

    // a function which colors colors the graph
    // this is run in parallel by the threads
    void pseudo_color(int ind1, int ind2, vector<int> &color){
        // legally color all the vertices from ind1 to ind2
        for(int i = ind1 ; i <= ind2 ; i++){
            vector<bool> color_nbr(n, false);
            for(int nbr : adj[i]){
                // if a neighbour (in this block) has already been color
                // then ensure this gets the next smallest available number
                if(nbr >= ind1 && nbr < i){
                    color_nbr[color[nbr]] = true;
                }
            }

            // find the smallest available color
            for(int j = 0 ; j < n ; j++){
                if(color_nbr[j] == false){
                    color[i] = j;
                    break;
                }
            }
        }
    }

    // Replaced the shared 'set' with a private 'vector' for this specific thread
    void check_clash_lockfree(int ind1, int ind2, const vector<int> &color, vector<int> &local_clashing) {
        for(int i = ind1 ; i <= ind2 ; i++){
            for(int nbr : adj[i]){
                // If a clash is detected
                if(color[i] == color[nbr]){
                    // Only the vertex with the smaller ID gets submitted for re-coloring.
                    // This guarantees no other thread will submit this exact conflict,
                    // and naturally prevents duplicates without needing a std::set.
                    if(i < nbr){
                        local_clashing.push_back(i);
                        // Optimization: Once 'i' is marked for re-coloring, 
                        // we don't need to check its other neighbors.
                        break; 
                    }
                    // If i > nbr, we do nothing. The thread processing 'nbr' will handle it.
                }
            }
        }
}

    void run_GM(){
        // define a vector of colors
        vector<int> color(n, -1);

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
            threads[i] = thread(&GM_lockfree::pseudo_color, this, ind1, ind2, ref(color));

            // update the current index
            cur_ind = ind2 + 1;
        }

        // wait for the threads to finish coloring
        for(int i = 0 ; i < p ; i ++){
            threads[i].join();
        }

        // PHASE 2 - check wrong colors (LOCK-FREE)

        // 1. Create a separate vector for each thread to write to safely
        vector<vector<int>> thread_clashes(p);

        cur_ind = 0;
        extra_vertices = n % p;

        for(int i = 0 ; i < p ; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){
                extra_vertices--;
                ind2++;
            }
            
            // 2. Pass the thread's specific local vector by reference
            threads[i] = thread(&GM_lockfree::check_clash_lockfree, this, ind1, ind2, cref(color), ref(thread_clashes[i]));

            cur_ind = ind2 + 1;
        }

        // 3. Wait for all threads to finish
        for(int i = 0 ; i < p ; i++){
            threads[i].join();
        }

        // 4. Flatten the local vectors into a single contiguous list for Phase 3
        vector<int> final_clashing;
        for(int i = 0; i < p; i++){
            final_clashing.insert(final_clashing.end(), thread_clashes[i].begin(), thread_clashes[i].end());
        }

        // PHASE 3 - color the vertices sequentially
        // (Update your phase 3 to loop over 'vector<int> final_clashing' instead of the set)
        for(int u : final_clashing){
            // ... rest of your Phase 3 MEX logic remains exactly the same ...
            vector<int> nbr_colors;
            for(int nbr : adj[u]){
                nbr_colors.push_back(color[nbr]);
            }
            sort(nbr_colors.begin(), nbr_colors.end());
            int mex = 0;
            for(int i : nbr_colors){
                if(mex==i){
                    mex++;
                }else if(mex < i){
                    break;
                }
            }

            color[u] = mex;

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
    GM_lockfree(vector<vector<int>> adj, int p){
        n = adj.size();
        this->adj = adj;
        this->p = p;

        run_GM();
    }

    int getNoOfColors(){
        return noOfColors;
    }

};