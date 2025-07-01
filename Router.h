//
// Created by Kyle C on 6/24/25.
//

#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include "TransitData.h"
#include "Types.h"

class Router {
public:
    explicit Router(TransitData& data) : data(data) {}
    State multiObjectiveDijkstra(const std::string& start, const std::string& end, double alpha, double beta, double gamma);

private:
    TransitData& data;
};

#endif
