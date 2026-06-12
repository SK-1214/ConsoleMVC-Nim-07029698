#pragma once
#include <string>

struct ProductionJob {
    int         orderId;
    std::string sampleId;
    int         actualQty;   // ceil(shortage / (yield * 0.9))
    int         totalTime;   // avgProductionTime * actualQty
};
