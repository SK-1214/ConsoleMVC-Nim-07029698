#pragma once
#include <string>
#include "Sample.h"

enum class StockStatus { SURPLUS, SHORTAGE, DEPLETED };

std::string stockStatusToString(StockStatus s);

struct SampleStockInfo {
    Sample      sample;
    int         pendingQty;     // total quantity in active orders (PRODUCING)
    StockStatus stockStatus;
};
