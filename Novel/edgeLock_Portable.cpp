#include<iostream>
#include<vector>
#include<thread>
#include<set>
#include<mutex>
#include<algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace std;

class TicketLock {
    std::atomic<size_t> next_ticket{0};
    std::atomic<size_t> now_serving{0};

public:
    void lock() {
        // 1. Take a ticket
        size_t my_ticket = next_ticket.fetch_add(1, std::memory_order_relaxed);

        // 2. Efficiently wait for our turn
        // This puts the thread to sleep in the kernel until 'now_serving' changes
        while (true) {
            size_t current_serving = now_serving.load(std::memory_order_acquire);
            if (current_serving == my_ticket) break;
            
            // Wait for now_serving to change from its current value
            now_serving.wait(current_serving);
        }
    }

    void unlock() {
        // 3. Increment and wake only ONE waiter
        now_serving.fetch_add(1, std::memory_order_release);
        now_serving.notify_all(); // Note: notify_all is used because tickets are unique
    }
};

class edgeLock_Portable {
private:
    TicketLock edge_queue_lock;

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

            // an indicator to mark cross edge
            bool isCrossEdge = false;

            for(int nbr : adj[i]){
                // edges within block
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
                        edge_queue_lock.lock();
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
                edge_queue_lock.unlock();
            }
        }
    }

    // This helper function finds the start/end vertex indices for each thread
    vector<pair<int, int>> get_edge_balanced_chunks() {
        vector<pair<int, int>> chunks;
        long long total_edges = 0;
        for (int i = 0; i < n; i++) total_edges += adj[i].size();

        long long target_per_thread = total_edges / p;
        int current_start = 0;
        long long current_sum = 0;

        for (int i = 0; i < n; i++) {
            current_sum += adj[i].size();
            // If this thread has reached its edge quota, or it's the last vertex
            if (current_sum >= target_per_thread && chunks.size() < (size_t)p - 1) {
                chunks.push_back({current_start, i});
                current_start = i + 1;
                current_sum = 0;
            }
        }
        // Final chunk for the remaining vertices
        chunks.push_back({current_start, n - 1});
        return chunks;
    }

    void run_GM(){
        // define a vector of colors
        color.assign(n, -1);

        vector<pair<int, int>> chunks = get_edge_balanced_chunks();

        // PHASE 1 - pseudo graph coloring

        // each thread will color a subset of edges from the graph
        // each thread will have approximatrly edges/p edges

        vector<thread> threads(p);
        for(int i = 0 ; i < p; i++){

            threads[i] = thread(&edgeLock_Portable::pseudo_color, this, chunks[i].first, chunks[i].second, ref(color));

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
    edgeLock_Portable (vector<vector<int>> adj, int p) {
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