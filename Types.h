#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

struct Edge {
    std::string to;
    double time;
    double fare;
    std::string route_id;
};

struct State {
    double totalCost;
    double fare;
    double time;
    int transfers;
    std::string node;
    std::vector<std::string> path;
    std::string lastRoute;

    bool operator>(const State& other) const {
        return totalCost > other.totalCost;
    }
};

struct FareProduct {
    double amount;
    std::string currency;
};

struct TimedStop {
    std::string trip_id;
    std::string stop_id;
    int stop_sequence;
    std::string arrival_time;
};

struct Mode {
    std::string label;
    double alpha, beta, gamma;
    State result;
};

#endif
