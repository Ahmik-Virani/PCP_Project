#include <os/lock.h> // The header for Apple-specific locks
#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>
#include <atomic>

using namespace std;

class edgeLock_Naive {
private:
    // This is just a 32-bit integer under the hood, making it very cache-friendly
    os_unfair_lock edge_lock = OS_UNFAIR_LOCK_INIT;

    int n;  // The number of vertices
    vector<vector<int>> adj; // The adjacency Graph
    int p; // The number of processes/threads

    // colors vector
    vector<int> color;

    int noOfColors; // The number of colors used to color the graph

    // a function which colors colors the graph
    // this is run in parallel by the threads
    void pseudo_color(int ind1, int ind2, vector<int> &color){
        // legally color all the vertices from ind1 to ind2
        for(int i = ind1 ; i <= ind2 ; i++){
            vector<bool> color_nbr(n, false);

            // an indicator to mark cross edge
            bool isCrossEdge = false;

            for(int nbr : adj[i]){
                // edges within block
                // if within the same block
                if(nbr >= ind1 && nbr <= ind2){
                    // [TODO] optimize
                    if(nbr < i)
                        color_nbr[color[nbr]] = true;
                    else {} // do nothing because not yet colored
                }
                // [NOVELTY] also check cross edges
                else{
                    // lock
                    if(!isCrossEdge){
                        os_unfair_lock_lock(&edge_lock);
                        isCrossEdge = true;
                    }
                    int nbr_color_to_check;
                    nbr_color_to_check = color[nbr];
                    if(nbr_color_to_check != -1)
                        color_nbr[nbr_color_to_check] = true;
                }
            }

            // find the smallest available color
            for(int j = 0 ; j < n ; j++){
                if(color_nbr[j] == false){
                    color[i] = j;
                    break;
                }
            }

            if(isCrossEdge){
                os_unfair_lock_unlock(&edge_lock);
            }
        }
    }

    void run_GM(){
        // define a vector of colors
        color.assign(n, -1);

        // PHASE 1 - pseudo graph coloring

        // each thread will color a subset of edges from the graph
        // each thread will have approximatrly edges/p edges

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
            threads[i] = thread(&edgeLock_Naive::pseudo_color, this, ind1, ind2, ref(color));

            // update the current index
            cur_ind = ind2 + 1;
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
    edgeLock_Naive (vector<vector<int>> adj, int p) {
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