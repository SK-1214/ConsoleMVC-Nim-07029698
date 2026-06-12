#pragma once
#include <vector>
#include "model/Order.h"
#include "model/StockInfo.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"

class MonitorController {
public:
    MonitorController(OrderRepository& orderRepo, SampleRepository& sampleRepo);

    std::vector<Order>           getOrdersByStatus(OrderStatus status) const;
    std::vector<SampleStockInfo> getStockInfo() const;

private:
    OrderRepository&  orderRepo_;
    SampleRepository& sampleRepo_;
};
