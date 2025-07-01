//
// Created by Kyle C on 6/24/25.
//

#ifndef TRANSITDATA_H
#define TRANSITDATA_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "Types.h"


using namespace std;

class TransitData {
public:
    unordered_map<string, vector<Edge>> graph;
    unordered_map<string, string> stopIdToName, nameToStopId, codeToStopId;
    unordered_map<string, string> tripToRoute;
    unordered_map<string, FareProduct> fareProducts;
    unordered_map<string, string> platformToStation;
    unordered_map<string, string> areaPairToFareProduct;
    unordered_map<string, string> stopIdToZone;
    vector<TimedStop> stopTimes;
    unordered_map<string, string> areaPairToFareId;
    unordered_map<string, double> fareIdToPrice;
    unordered_map<string, int> stopIdUsageCount;

    bool load(const string& folder, const string& zipfile);
    void parseStops(const string& path);
    void parseFareAttributes(const string& path);
    void parseFareRules(const string& path);
    void parseTrips(const string& path);
    void parseStopTimes(const string& path);
    void buildGraph();

private:
    void parseFile(const string& path, function<void(stringstream&)> handler);
    static string trim(const string& s);
    static bool isValidNumber(const string& s);
    static int parseTime(const string& t);
};

#endif
