//
// Created by Kyle C on 6/24/25.
//

#include "TransitData.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;

// Unzips and parse GTFS files
bool TransitData::load(const string& folder, const string& zipfile) {
    string cmd = "unzip -o " + folder + "/" + zipfile + " -d " + folder + "/ > /dev/null 2>&1"; // unzip gtfs file

    if (system(cmd.c_str()) != 0) {
        cerr << "Failed to unzip GTFS archive.\n";
        return false;
    }

    // conventional file format of GTFS
    parseStops(folder + "/stops.txt");
    parseFareAttributes(folder + "/fare_attributes.txt");
    parseFareRules(folder + "/fare_rules.txt");
    parseTrips(folder + "/trips.txt");
    parseStopTimes(folder + "/stop_times.txt");
    buildGraph();
    system("rm data/*.txt"); // Deletes unzipped .txt files (comment out if need to inspect .txt files)

    return true;
}

// Helper f(x) to instream a CSV file line by line
void TransitData::parseFile(const string& path, function<void(stringstream&)> handler) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "File cannot be opened. " << path << endl;
        return;
    }
    string line;
    getline(file, line);  // skip header
    while (getline(file, line)) {
        stringstream ss(line);
        handler(ss);
    }
}

// Parse stops.txt and map stop details (ID, name, zone, platform mapping)
void TransitData::parseStops(const string& path) {
    parseFile(path, [&](stringstream& ss) {
        string id, code, name, desc, lat, lon, zone, plc, location_type, parent, platform;
        getline(ss, id, ','); getline(ss, code, ','); getline(ss, name, ',');
        getline(ss, desc, ','); getline(ss, lat, ','); getline(ss, lon, ',');
        getline(ss, zone, ','); getline(ss, plc, ','); getline(ss, location_type, ',');
        getline(ss, parent, ','); getline(ss, platform, ',');

        id = trim(id); code = trim(code); name = trim(name);
        location_type = trim(location_type); parent = trim(parent); zone = trim(zone);

        if (!zone.empty()) stopIdToZone[id] = zone;
        if (!parent.empty()) platformToStation[id] = parent;
        stopIdToName[id] = name;  // save name for all stops

        if (location_type == "1") {
            nameToStopId[name] = id;
            codeToStopId[code] = id;
        }

    });
}

// Parse file: fare_attributes.txt (fare_id -> price)
void TransitData::parseFareAttributes(const string& path) {
    parseFile(path, [&](stringstream& ss) {
        string fare_id, price_str, currency, payment_method, transfers, duration;
        getline(ss, fare_id, ','); getline(ss, price_str, ','); getline(ss, currency, ',');
        getline(ss, payment_method, ','); getline(ss, transfers, ','); getline(ss, duration, ',');

        fare_id = trim(fare_id); price_str = trim(price_str); currency = trim(currency);

        if (TransitData::isValidNumber(price_str)) {
            fareProducts[fare_id] = {stod(price_str), currency};
        }
    });
}

// Parse file: fare_rules.txt (zone pair -> fare_id)
void TransitData::parseFareRules(const string& path) {
    parseFile(path, [&](stringstream& ss) {
        string fare_id, route_id, origin_id, destination_id, contains_id;
        getline(ss, fare_id, ','); getline(ss, route_id, ','); getline(ss, origin_id, ',');
        getline(ss, destination_id, ','); getline(ss, contains_id, ',');

        fare_id = trim(fare_id); origin_id = trim(origin_id); destination_id = trim(destination_id);

        if (!origin_id.empty() && !destination_id.empty() && !fare_id.empty()) {
            areaPairToFareProduct[origin_id + "_" + destination_id] = fare_id;
        }
    });
}

// Parse file: trips.txt (trip_id -> route_id)
void TransitData::parseTrips(const string& path) {
    parseFile(path, [&](stringstream& ss) {
        string route_id, service_id, trip_id;
        getline(ss, route_id, ','); getline(ss, service_id, ','); getline(ss, trip_id, ',');
        if (!trip_id.empty() && !route_id.empty()) {
            tripToRoute[trip_id] = route_id;
        }
    });
}

// Parse stop_times.txt into structured stop sequences per trip
void TransitData::parseStopTimes(const string& path) {
    parseFile(path, [&](stringstream& ss) {
        string trip_id, arrival, departure, stop_id, seq_str;
        getline(ss, trip_id, ','); getline(ss, arrival, ','); getline(ss, departure, ',');
        getline(ss, stop_id, ','); getline(ss, seq_str, ',');
        try {
            int seq = stoi(seq_str);
            stopTimes.push_back({trip_id, stop_id, seq, arrival});
        } catch (...) {}
    });
}

// Build bidirectional graph from stop sequences, assigning time, fare, and route_id
void TransitData::buildGraph() {
    sort(stopTimes.begin(), stopTimes.end(), [](const TimedStop& a, const TimedStop& b) {
        return tie(a.trip_id, a.stop_sequence) < tie(b.trip_id, b.stop_sequence);
    });

    auto remap = [&](const string& id) {
        return platformToStation.count(id) ? platformToStation[id] : id;
    };

    for (size_t i = 1; i < stopTimes.size(); ++i) {
        if (stopTimes[i].trip_id != stopTimes[i - 1].trip_id) continue;

        string from = remap(stopTimes[i - 1].stop_id);
        string to   = remap(stopTimes[i].stop_id);
        string trip_id = stopTimes[i].trip_id;
        string route_id = tripToRoute.count(trip_id) ? tripToRoute[trip_id] : "";

        // Calc duration b/w two stops (default to 5 mins if unk)
        int t1 = parseTime(stopTimes[i - 1].arrival_time);
        int t2 = parseTime(stopTimes[i].arrival_time);
        double duration = (t1 >= 0 && t2 >= 0) ? (t2 - t1) / 60.0 : -1;
        if (duration <= 0.0) duration = 5.0;

        double fare = 0.50; // flat fare per segment

        // Add edges in both directions to make graph bidirectional
        graph[from].push_back({to, duration, fare, route_id});
        graph[to].push_back({from, duration, fare, route_id});
    }
}

// Remove quotes from str
string TransitData::trim(const string& s) {
    return (s.size() >= 2 && s.front() == '"' && s.back() == '"') ? s.substr(1, s.size() - 2) : s;
}

// Check if str can be converted to double
bool TransitData::isValidNumber(const string& s) {
    try { stod(s); return true; } catch (...) { return false; }
}

// Parse time str from HH:MM:SS to seconds
int TransitData::parseTime(const string& t) {
    int h, m, s;
    if (sscanf(t.c_str(), "%d:%d:%d", &h, &m, &s) != 3) return -1;
    return h * 3600 + m * 60 + s;
}
