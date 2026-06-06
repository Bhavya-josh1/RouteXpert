#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <queue>
#include <stack>
#include <limits>
#include <algorithm>
using namespace std;

// ─── GRAPH DATA ───────────────────────────────────────────────
// nodes = list of all delivery locations
// adj   = for each node, list of its neighbors and distances
vector<string> names;
vector<double> xs, ys;
vector<vector<pair<int,double>>> adj;
int numNodes = 0;

// ─── GRAPH SETUP FUNCTIONS ────────────────────────────────────

void addNode(string name, double x, double y) {
    names.push_back(name);
    xs.push_back(x);
    ys.push_back(y);
    adj.push_back(vector<pair<int,double>>());
    numNodes++;
}

double getDist(int i, int j) {
    double dx = xs[i] - xs[j];
    double dy = ys[i] - ys[j];
    return sqrt(dx*dx + dy*dy);
}

void addEdge(int i, int j) {
    double d = getDist(i, j);
    adj[i].push_back({j, d});
    adj[j].push_back({i, d});
}

void buildCompleteGraph() {
    // connect every location to every other location
    // needed so TSP can go from any point to any point
    for(int i = 0; i < numNodes; i++)
        for(int j = i+1; j < numNodes; j++)
            addEdge(i, j);
}

// ─── BFS ──────────────────────────────────────────────────────
// explores graph level by level using a queue
// use: check if all locations are reachable from warehouse
vector<int> bfs(int start) {
    vector<bool> visited(numNodes, false);
    vector<int> order;
    queue<int> q;

    visited[start] = true;
    q.push(start);

    while(!q.empty()) {
        int curr = q.front();
        q.pop();
        order.push_back(curr);

        for(auto neighbor : adj[curr]) {
            if(!visited[neighbor.first]) {
                visited[neighbor.first] = true;
                q.push(neighbor.first);
            }
        }
    }
    return order;
}

// ─── DFS ──────────────────────────────────────────────────────
// explores graph by going deep first using a stack
// use: find all locations connected to warehouse
vector<int> dfs(int start) {
    vector<bool> visited(numNodes, false);
    vector<int> order;
    stack<int> s;

    s.push(start);

    while(!s.empty()) {
        int curr = s.top();
        s.pop();

        if(!visited[curr]) {
            visited[curr] = true;
            order.push_back(curr);

            for(auto neighbor : adj[curr])
                if(!visited[neighbor.first])
                    s.push(neighbor.first);
        }
    }
    return order;
}

// ─── DIJKSTRA ─────────────────────────────────────────────────
// finds shortest path between two locations
// greedy - always picks the nearest unvisited node next
// time complexity: O((V+E) log V)
vector<int> dijkstra(int start, int end) {
    vector<double> dist(numNodes, numeric_limits<double>::infinity());
    vector<int> prev(numNodes, -1);
    vector<bool> visited(numNodes, false);

    // min heap - {distance, node}
    priority_queue<pair<double,int>,
                   vector<pair<double,int>>,
                   greater<pair<double,int>>> pq;

    dist[start] = 0;
    pq.push({0, start});

    while(!pq.empty()) {
        int curr = pq.top().second;
        pq.pop();

        if(visited[curr]) continue;
        visited[curr] = true;

        for(auto neighbor : adj[curr]) {
            int next = neighbor.first;
            double weight = neighbor.second;

            if(dist[curr] + weight < dist[next]) {
                dist[next] = dist[curr] + weight;
                prev[next] = curr;
                pq.push({dist[next], next});
            }
        }
    }

    // rebuild path from end to start using prev array
    vector<int> path;
    for(int at = end; at != -1; at = prev[at])
        path.push_back(at);
    reverse(path.begin(), path.end());
    return path;
}

// ─── GREEDY TSP ───────────────────────────────────────────────
// starts at warehouse, always goes to nearest unvisited location
// fast but not always optimal - O(n²)
vector<int> greedyTSP(int start) {
    vector<bool> visited(numNodes, false);
    vector<int> path;

    int curr = start;
    visited[curr] = true;
    path.push_back(curr);

    for(int step = 0; step < numNodes-1; step++) {
        double bestDist = numeric_limits<double>::infinity();
        int bestNext = -1;

        for(int next = 0; next < numNodes; next++) {
            if(!visited[next] && getDist(curr, next) < bestDist) {
                bestDist = getDist(curr, next);
                bestNext = next;
            }
        }

        visited[bestNext] = true;
        path.push_back(bestNext);
        curr = bestNext;
    }

    path.push_back(start); // return to warehouse
    return path;
}

// ─── HELD-KARP TSP (DP) ───────────────────────────────────────
// finds the OPTIMAL delivery route using dynamic programming
// state: dp[visited locations as bitmask][current location]
// time complexity: O(2^n * n²) - works well up to 15 locations
vector<int> heldKarpTSP() {
    int n = numNodes;
    int FULL = (1 << n) - 1; // bitmask when all nodes visited

    // dp[mask][i] = min distance to reach node i
    //               having visited nodes in mask
    vector<vector<double>> dp(1<<n, vector<double>(n, numeric_limits<double>::infinity()));
    vector<vector<int>> parent(1<<n, vector<int>(n, -1));

    dp[1][0] = 0; // start at node 0 (warehouse), only it is visited

    for(int mask = 1; mask <= FULL; mask++) {
        for(int curr = 0; curr < n; curr++) {
            if(!(mask & (1<<curr))) continue;
            if(dp[mask][curr] == numeric_limits<double>::infinity()) continue;

            for(int next = 0; next < n; next++) {
                if(mask & (1<<next)) continue; // already visited

                int newMask = mask | (1<<next);
                double newDist = dp[mask][curr] + getDist(curr, next);

                if(newDist < dp[newMask][next]) {
                    dp[newMask][next] = newDist;
                    parent[newMask][next] = curr;
                }
            }
        }
    }

    // find best ending node
    double bestDist = numeric_limits<double>::infinity();
    int lastNode = -1;
    for(int i = 1; i < n; i++) {
        double total = dp[FULL][i] + getDist(i, 0);
        if(total < bestDist) {
            bestDist = total;
            lastNode = i;
        }
    }

    // rebuild path
    vector<int> path;
    int mask = FULL;
    int curr = lastNode;
    while(curr != -1) {
        path.push_back(curr);
        int prev = parent[mask][curr];
        mask = mask ^ (1<<curr);
        curr = prev;
    }
    reverse(path.begin(), path.end());
    path.push_back(0); // return to warehouse
    return path;
}

// ─── CALCULATE TOTAL DISTANCE OF A PATH ───────────────────────
double pathDistance(vector<int> path) {
    double total = 0;
    for(int i = 0; i < path.size()-1; i++)
        total += getDist(path[i], path[i+1]);
    return total;
}

// ─── WRITE OUTPUT TO JSON ─────────────────────────────────────
// C++ writes result here, JavaScript reads it to draw the route
void writeOutput(vector<int> path, string algorithm, double totalDist) {
    ofstream file("data/output.json");

    file << "{\n";
    file << "  \"algorithm\": \"" << algorithm << "\",\n";
    file << "  \"totalDistance\": " << totalDist << ",\n";

    // write node coordinates
    file << "  \"nodes\": [\n";
    for(int i = 0; i < numNodes; i++) {
        file << "    {\"name\": \"" << names[i] << "\", "
             << "\"x\": " << xs[i] << ", "
             << "\"y\": " << ys[i] << "}";
        if(i < numNodes-1) file << ",";
        file << "\n";
    }
    file << "  ],\n";

    // write the route as sequence of node indices
    file << "  \"path\": [";
    for(int i = 0; i < path.size(); i++) {
        file << path[i];
        if(i < path.size()-1) file << ", ";
    }
    file << "]\n";
    file << "}";

    file.close();
}
