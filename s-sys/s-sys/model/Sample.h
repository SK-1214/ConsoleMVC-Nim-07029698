#pragma once
#include <string>

struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime;  // average time to produce one unit
    double      yield;              // 0.0 ~ 1.0
    int         stock = 0;          // current inventory
};
