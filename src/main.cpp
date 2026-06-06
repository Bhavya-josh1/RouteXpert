#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// include graph.cpp directly so we can use all its functions
// this removes the need for a .h header file
#include "graph.cpp"

int main() {
    // ─── READ INPUT FROM FILE ─────────────────────────────────
    // JavaScript writes node coordinates and algorithm choice here
    ifstream inputFile("data/input.txt");

    if(!inputFile.is_open()) {
        cout << "Error: could not open data/input.txt\n";
        return 1;
    }

    string algorithm;
    int n;
    inputFile >> algorithm >> n;

    for(int i = 0; i < n; i++) {
        string name;
        double x, y;
        inputFile >> name >> x >> y;
        addNode(name, x, y);
    }

    inputFile.close();
    buildCompleteGraph();

    // ─── RUN SELECTED ALGORITHM ───────────────────────────────
    vector<int> path;
    string algoName;

    if(algorithm == "BFS") {
        path = bfs(0);
        algoName = "BFS (Breadth First Search)";
    }
    else if(algorithm == "DFS") {
        path = dfs(0);
        algoName = "DFS (Depth First Search)";
    }
    else if(algorithm == "DIJKSTRA") {
        // dijkstra finds shortest path from warehouse to last location
        path = dijkstra(0, n-1);
        algoName = "Dijkstra Shortest Path";
    }
    else if(algorithm == "GREEDY") {
        path = greedyTSP(0);
        algoName = "Greedy Nearest Neighbor TSP";
    }
    else if(algorithm == "HELDKARP") {
        path = heldKarpTSP();
        algoName = "Held-Karp Optimal TSP (DP)";
    }
    else {
        // default to greedy if nothing selected
        path = greedyTSP(0);
        algoName = "Greedy Nearest Neighbor TSP";
    }

    // ─── CALCULATE TOTAL DISTANCE AND WRITE OUTPUT ────────────
    double totalDist = pathDistance(path);
    writeOutput(path, algoName, totalDist);

    cout << "Algorithm: " << algoName << "\n";
    cout << "Total Distance: " << totalDist << "\n";
    cout << "Route: ";
    for(int i = 0; i < path.size(); i++) {
        cout << names[path[i]];
        if(i < path.size()-1) cout << " -> ";
    }
    cout << "\nOutput written to data/output.json\n";

    return 0;
}
