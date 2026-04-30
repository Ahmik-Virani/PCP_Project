// This is GPT Generated
// Credits: Gemini

#include <vector>
#include <iostream>

using namespace std;

class CheckColoring {
public:
    /**
     * Validates if the graph coloring is proper.
     * @param adj The adjacency list representing the graph.
     * @param color The array/vector of colors assigned to each vertex.
     * @return true if the coloring is valid, false otherwise.
     */
    static bool isValid(const vector<vector<int>>& adj, const vector<int>& color) {
        int n = adj.size();

        // Ensure the color vector matches the number of vertices
        if (color.size() != (size_t)n) {
            return false;
        }

        for (int i = 0; i < n; i++) {
            // 1. Check if the vertex was actually colored
            // -1 is the standard "uncolored" indicator in your GM classes
            if (color[i] < 0) {
                return false;
            }

            // 2. Check all neighbors of vertex i
            for (int nbr : adj[i]) {
                // If a neighbor has the same color, the coloring is invalid
                if (color[i] == color[nbr]) {
                    return false;
                }
            }
        }

        // If no violations were found after checking all vertices and edges
        return true;
    }
};