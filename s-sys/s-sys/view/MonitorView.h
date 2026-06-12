#pragma once
#include <vector>
#include "model/Order.h"
#include "model/StockInfo.h"

class MonitorView {
public:
    void displayOrdersByStatus(const std::vector<Order>& orders, OrderStatus status) const;
    void displayStockInfo(const std::vector<SampleStockInfo>& info) const;
};
