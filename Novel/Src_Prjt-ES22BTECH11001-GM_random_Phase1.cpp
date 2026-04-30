#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>
#include <random>

using namespace std;

class GM_random_Phase1 {
private:    
    // lock for the clashing set used in phase 2
    mutex clashing_set_lock;

    // to generate random numbers
    mt19937 generator;
    uniform_int_distribution<int> distribution;


    int n;  // The number of vertices
    vector<vector<int>> adj; // The adjacency Graph
    int p; // The number of processes/threads

    vector<int> color;

    int noOfColors; // The number of colors used to color the graph

    // a function which colors colors the graph
    // this is run in parallel by the threads
    void pseudo_color(int ind1, int ind2, vector<int> &color){
        // legally color all the vertices from ind1 to ind2
        for(int i = ind1 ; i <= ind2 ; i++){
            // randomly color
            color[i] = distribution(generator);
        }
    }

    void check_clash(int ind1, int ind2, const vector<int> &color, set<int> &clashing){

        for(int i = ind1 ; i <= ind2 ; i++){
            for(int nbr : adj[i]){
                if(color[i]==color[nbr]){
                    clashing_set_lock.lock();
                    clashing.insert(min(i, nbr));
                    clashing_set_lock.unlock();
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
            threads[i] = thread(&GM_random_Phase1::pseudo_color, this, ind1, ind2, ref(color));

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
        set<int> clashing;
        for(int i = 0 ; i < p ; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){
                extra_vertices--;
                ind2++;
            }
            threads[i] = thread(&GM_random_Phase1::check_clash, this, ind1, ind2, cref(color), ref(clashing));

            // update the current index
            cur_ind = ind2 + 1;
        }

        // wait for the threads to finish their checks
        for(int i = 0 ; i < p ; i++){
            threads[i].join();
        }

        // PHASE 3 - color the vertices sequentially
        for(int u : clashing){
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

    void run_sequential_GM(){
        // define a vector of colors
        color.assign(n, -1);

        // PHASE 1 - pseudo graph coloring

        int cur_ind = 0;
        int extra_vertices = n%p;

        for(int i = 0 ; i < p; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){
                extra_vertices--;
                ind2++;
            }
            pseudo_color(ind1, ind2, color);

            // update the current index
            cur_ind = ind2 + 1;
        }

        // we will not have any wrong colors

        // but to be consistent, do this phase
        // PHASE 2 - check wrong colors
        
        // we have a set storing the vertices who have same colors adjacent to each other
        cur_ind = 0;
        extra_vertices = n%p;
        set<int> clashing;
        for(int i = 0 ; i < p ; i++){
            int ind1 = cur_ind;
            int ind2 = cur_ind + n/p - 1;
            if(extra_vertices){
                extra_vertices--;
                ind2++;
            }
            check_clash(ind1, ind2, color, clashing);

            // update the current index
            cur_ind = ind2 + 1;
        }

        // again will not be needed
        // but just kept for a sanity check

        // PHASE 3 - color the vertices sequentially
        for(int u : clashing){
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
    // p is 1 for sequential
    GM_random_Phase1(vector<vector<int>> adj, int p){
        n = adj.size();
        this->adj = adj;
        this->p = p;

        int bound = 0;
        for(int i = 0 ; i < n ; i++){
            bound = max(bound, (int)adj[i].size() + 1);
        }

        distribution = uniform_int_distribution<int>(0, bound - 1);

        if(p != 1)
            run_GM();
        else if(p==1)
            run_sequential_GM();
    }

    int getNoOfColors(){
        return noOfColors;
    }

    vector<int> return_color(){
        return color;
    }
};