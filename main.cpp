#include <iostream>
#include "TransitData.h"
#include "Router.h"
#include <chrono>

/**
 * Parameters:
 *  Fare depends on stop count b/c most transportations are flat fee.
 *  Fare ($0.50/stop): increases with path length.
 *  Time (min): accumulated per segment from GTFS arrival times.
 *  Transfers: increments on route changes.
 *  Tradeoff f(x): Score = α * fare + β * time + γ * transfers.
**/

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: ./Final2 <start_stop_id|name|code> <end_stop_id|name|code>\n";
        return 1;
    }

    string start = argv[1];
    string end   = argv[2];

    // Load GTFS data (including fare rules and graph)
    TransitData data;
    if (!data.load("data", "gtfs_london.zip")) { // Change filename, if needed
        cerr << "Failed to load GTFS data.\n";
        return 1;
    }

    // Resolve stop input (ID, name, or code) to stop_id in the graph
    auto resolve = [&](const string& input) -> string {
        if (data.graph.count(input)) return input;
        if (data.nameToStopId.count(input) && data.graph.count(data.nameToStopId[input]))
            return data.nameToStopId[input];
        if (data.codeToStopId.count(input) && data.graph.count(data.codeToStopId[input]))
            return data.codeToStopId[input];
        return input;
    };

    start = resolve(start);
    end = resolve(end);

    if (!data.graph.count(start) || !data.graph.count(end)) {
        cerr << "Invalid start or end point.\n";
        return 1;
    }

    // Fare calculation from start to end stations
    string zone_start = data.stopIdToZone[start];
    string zone_end   = data.stopIdToZone[end];
    string key = zone_start + "_" + zone_end;
    double flatFare = 2.50;

    if (data.areaPairToFareId.count(key)) {
        string fare_id = data.areaPairToFareId[key];
        if (data.fareIdToPrice.count(fare_id)) {
            flatFare = data.fareIdToPrice[fare_id];
        }
    }

    // Multiple routing strategies (respective weights for α, β, γ)
    Router router(data);
    vector<Mode> modes = {
        {"Cheapest", 1.2, 0.1, 0.1},
        {"Fastest", 0.1, 0.8, 0.2},
        {"Balanced", 0.5, 0.5, 0.5}

    };

    // auto startPt = chrono::high_resolution_clock::now();

    // Run modified Dijkstra for each mode
    for (auto& mode : modes) {
        mode.result = router.multiObjectiveDijkstra(start, end, mode.alpha, mode.beta, mode.gamma);

        if (!mode.result.path.empty()) {
            // 🔁 Recalculate actual number of transfers using route_id transitions
            int transferCount = 0;
            string prevRoute = "";

            auto& path = mode.result.path;
            for (size_t i = 1; i < path.size(); ++i) {
                const string& from = path[i - 1];
                const string& to   = path[i];

                for (const Edge& edge : data.graph[from]) {
                    if (edge.to == to) {
                        string currRoute = edge.route_id;
                        if (!prevRoute.empty() && currRoute != prevRoute) {
                            transferCount++;
                        }
                        prevRoute = currRoute;
                        break;
                    }
                }
            }
            // Override incorrect transfer count and recalculate cost
            mode.result.transfers = transferCount;
            mode.result.totalCost = mode.alpha * mode.result.fare +
                                    mode.beta  * mode.result.time +
                                    mode.gamma * mode.result.transfers;
        }
    }

    // // Time measurement
    // auto endPt = chrono::high_resolution_clock::now();
    // auto duration = chrono::duration_cast<chrono::milliseconds>(endPt - startPt);
    // cout << "Runtime: " << duration.count() << " ms\n";

    // Table formatting
    cout << "\n*" << data.stopIdToName[start]
         << " (" << start << ") "
         << "to " << data.stopIdToName[end]
         << " (" << end << ")\n";
    cout << "Mode                   | Fare ($) | Time (min) | Transfers | Score \n";
    cout << "-------------------------------------------------------------------\n";

    for (const auto& mode : modes) {
        const auto& s = mode.result;
        if (s.path.empty()) {
            printf("%-22s |  N/A     |    N/A     |    N/A   |   N/A  \n", mode.label.c_str());
        } else {
            printf("%-22s |  %5.2f   |   %5.1f    |     %2d    |  %5.2f \n",
                   mode.label.c_str(), s.fare, s.time, s.transfers, s.totalCost);
        }
    }

    // Helper function to format station display as "Name #Number (StopID)"
    auto formatStop = [&](const string& stopId) {
        string fullName = data.stopIdToName.count(stopId) ? data.stopIdToName[stopId] : "(Unknown)";
        string stopNum = "(N/A)";
        size_t hashPos = fullName.find("#");
        if (hashPos != string::npos) {
            stopNum = fullName.substr(hashPos);
            fullName = fullName.substr(0, hashPos - 1); // remove trailing space
        }
        return fullName + " " + stopNum + " (" + stopId + ")";
    };

    // Print all stations visited in the Cheapest route
    auto& cheapest = modes[0];
    cout << "\nVisited stations in Cheapest Route:\n";
    string prevRoute = "";
    int countedTransfers = 0;

    // Traverse Cheapest path and detect transfers based on route_id changes
    for (size_t i = 1; i < cheapest.result.path.size(); ++i) {
        const string& from = cheapest.result.path[i - 1];
        const string& to   = cheapest.result.path[i];

        for (const Edge& edge : data.graph[from]) {
            if (edge.to == to) {
                string currRoute = edge.route_id.empty() ? "(unknown)" : edge.route_id;
                string fromFormatted = formatStop(from);
                string toFormatted   = formatStop(to);

                if (!prevRoute.empty() && currRoute != prevRoute) {
                    countedTransfers++;
                    cout << "TRANSFER: " << fromFormatted << "->" << toFormatted
                         << " (" << prevRoute << "  --->  " << currRoute << ")\n";
                } else {
                    cout << "ROUTE:    " << fromFormatted << "  --->  " << toFormatted
                         << " (" << currRoute << ")\n";
                }

                prevRoute = currRoute;
                break;
            }
        }
    }
    return 0;
}
