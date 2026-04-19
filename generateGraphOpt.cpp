#include <iostream>
#include <vector>
#include <random>
#include <cmath> // Added for log() and floor()

using namespace std;

class generateGraph {
private:
    int n;                      // number of vertices
    vector<vector<int>> adj;    // adjacency list
    
public:
    // constructor
    generateGraph(int n, double p) {
        this->n = n;
        adj.resize(n);

        // If probability is 0, just return the empty graph
        if (p <= 0.0) return; 
        
        // If probability is 1, it's a complete graph (handling this prevents log(0) errors)
        if (p >= 1.0) {
            for(int i = 0; i < n; i++) {
                for(int j = i + 1; j < n; j++) {
                    adj[i].push_back(j);
                    adj[j].push_back(i);
                }
            }
            return;
        }

        random_device rd;
        mt19937 gen(rd());
        
        // Note: We use a tiny number above 0 because log(0) is undefined
        uniform_real_distribution<double> dis(0.000000001, 1.0);

        // --- Batagelj and Brandes O(E) Algorithm ---
        
        int v = 1;
        int w = -1;
        double log_one_minus_p = log(1.0 - p); // Precompute this for speed

        while (v < n) {
            double r = dis(gen);
            
            // Calculate how many non-edges to SKIP using the geometric distribution
            int skip = floor(log(1.0 - r) / log_one_minus_p);
            
            w = w + 1 + skip;

            // If 'w' overflows beyond the current vertex 'v', 
            // wrap it around to the next vertices
            while (w >= v && v < n) {
                w = w - v;
                v = v + 1;
            }

            // If we haven't exceeded the graph size, place the edge!
            if (v < n) {
                adj[v].push_back(w);
                adj[w].push_back(v);
            }
        }
    }

    vector<vector<int>> getGraph() {
        return adj;
    }
};