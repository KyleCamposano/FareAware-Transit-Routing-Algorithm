//
// Created by Kyle C on 6/24/25.
//

#include "Router.h"
#include <queue>
#include <limits>
#include <unordered_map>
#include <vector>

using namespace std;

// Modified Dijkstra algorithm with a f(x) that weighs trip fare, duration, and transfers to optimize cost.
State Router::multiObjectiveDijkstra(const string& start, const string& end, double alpha, double beta, double gamma) {
    priority_queue<State, vector<State>, greater<State>> pq; // min heap PQ sorted by totalCost
    unordered_map<string, bool> visited;

    pq.push({0.0, 0.0, 0.0, 0, start, {}, ""});

    while (!pq.empty()) {
        State current = pq.top(); pq.pop();

        // Skip if already visited (finalized)
        if (visited[current.node]) continue;
        visited[current.node] = true;

        // Extend the current path
        vector<string> newPath = current.path;
        newPath.push_back(current.node);

        // Return full path and cost calc
        if (current.node == end) {
            return {current.totalCost, current.fare, current.time, current.transfers, current.node, newPath, current.lastRoute};
        }

        // Explore all outgoing edges from current stop
        for (const Edge& edge : data.graph[current.node]) {
            if (visited[edge.to]) continue;

            double newTime = current.time + edge.time; // Update cumulative calc for this edge

            // Count as transfer (if route_id changes from previous segment)
            bool isTransfer = (!current.lastRoute.empty() && current.lastRoute != edge.route_id);
            int newTransfers = current.transfers + (isTransfer ? 1 : 0);

            double newFare = current.fare + edge.fare; // Fare per segment
            double newCost = alpha * newFare + beta * newTime + gamma * newTransfers; // Cost of path

            pq.push({newCost, newFare, newTime, newTransfers, edge.to, newPath, edge.route_id});
        }
    }
    return {numeric_limits<double>::infinity(), -1, -1, -1, "", {}, ""}; // No path found
}
