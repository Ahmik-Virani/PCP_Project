#include <iostream>
#include <vector>
#include <thread>
#include <set>
#include <atomic>
#include <algorithm>

using namespace std;

class GM_EdgeLock_Final {
private:
    int n, p;
    vector<vector<int>> adj;

    vector<int> color;
    int noOfColors;

    // Global atomic state (0 = free, 1 = locked)
    atomic<int> state{0};

    // ---------------- CHUNKING ----------------
    vector<pair<int,int>> get_edge_balanced_chunks() {
        vector<pair<int,int>> chunks;
        long long total_edges = 0;
        for (int i = 0; i < n; i++) total_edges += adj[i].size();

        long long target_per_thread = total_edges / p;
        int current_start = 0;
        long long current_sum = 0;

        for (int i = 0; i < n; i++) {
            current_sum += adj[i].size();
            if (current_sum >= target_per_thread && chunks.size() < (size_t)p - 1) {
                chunks.push_back({current_start, i});
                current_start = i + 1;
                current_sum = 0;
            }
        }
        chunks.push_back({current_start, n - 1});
        return chunks;
    }

    // ---------------- THREAD WORK ----------------
    void process_chunk(int L, int R) {

        // Worklist per thread
        set<int> workset;
        for (int i = L; i <= R; i++) workset.insert(i);

        while (!workset.empty()) {

            // iterate over set (IMPORTANT: no immediate retry bias)
            for (auto it = workset.begin(); it != workset.end(); ) {

                int i = *it;
                vector<bool> used(n, false);

                bool needsGlobal = false;

                // check if cross edge exists
                for (int nbr : adj[i]) {
                    if (!(nbr >= L && nbr <= R)) {
                        needsGlobal = true;
                        break;
                    }
                }

                // ---------------- CROSS EDGE ----------------
                if (needsGlobal) {

                    // try to acquire (NO SPIN)
                    if (state.exchange(1, memory_order_acquire) == 1) {
                        ++it;  // move on, don't retry immediately
                        continue;
                    }

                    // success → process node
                    for (int nbr : adj[i]) {
                        int c = color[nbr];
                        if (c != -1) used[c] = true;
                    }

                    for (int c = 0; c < n; c++) {
                        if (!used[c]) {
                            color[i] = c;
                            break;
                        }
                    }

                    // release
                    state.store(0, memory_order_release);

                    // remove node (SUCCESS)
                    it = workset.erase(it);
                }

                // ---------------- LOCAL EDGE ----------------
                else {
                    for (int nbr : adj[i]) {
                        if (nbr < i) {
                            int c = color[nbr];
                            if (c != -1) used[c] = true;
                        }
                    }

                    for (int c = 0; c < n; c++) {
                        if (!used[c]) {
                            color[i] = c;
                            break;
                        }
                    }

                    // remove node
                    it = workset.erase(it);
                }
            }
        }
    }

    // ---------------- MAIN ----------------
    void run_GM() {

        color.assign(n, -1);

        auto chunks = get_edge_balanced_chunks();

        vector<thread> threads;

        for (int i = 0; i < p; i++) {
            threads.emplace_back(&GM_EdgeLock_Final::process_chunk,
                                 this,
                                 chunks[i].first,
                                 chunks[i].second);
        }

        for (auto &t : threads) t.join();

        // compute number of colors
        noOfColors = 0;
        for (int i = 0; i < n; i++) {
            noOfColors = max(noOfColors, color[i]);
        }
        noOfColors++;
    }

public:
    GM_EdgeLock_Final(vector<vector<int>> adj, int p) {
        this->adj = adj;
        this->n = adj.size();
        this->p = p;

        run_GM();
    }

    int getNoOfColors() {
        return noOfColors;
    }

    vector<int> get_color() {
        return color;
    }
};