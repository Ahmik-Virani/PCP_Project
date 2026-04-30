// opt no 1

#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>
#include <atomic>

using namespace std;

class GM_lockfree_Phase2 {
private:    
    int n;  // The number of vertices
    vector<vector<int>> adj; // The adjacency Graph
    int p; // The number of processes/threads

    int noOfColors; // The number of colors used to color the graph

    vector<int> color;

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
            threads[i] = thread(&GM_lockfree_Phase2::pseudo_color, this, ind1, ind2, ref(color));

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
            threads[i] = thread(&GM_lockfree_Phase2::check_clash, this, ind1, ind2, cref(color), ref(is_clashing));

            // update the current index
            cur_ind = ind2 + 1;
        }

        // wait for the threads to finish their checks
        for(int i = 0 ; i < p ; i++){
            threads[i].join();
        }

        // PHASE 3 - color the vertices sequentially
        for(int i = 0; i < n; i++) {
            if(is_clashing[i].load()){
                vector<int> nbr_colors;
                for(int nbr : adj[i]){
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

                color[i] = mex;

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
    GM_lockfree_Phase2(vector<vector<int>> adj, int p){
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