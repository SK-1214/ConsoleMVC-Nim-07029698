#pragma once
#include <string>
#include <vector>
#include "model/Order.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"
#include "model/ProductionQueue.h"

class OrderController {
public:
    OrderController(OrderRepository& orderRepo, SampleRepository& sampleRepo,
                    ProductionQueue& productionQueue);

    // Returns new order id, or -1 on failure
    int placeOrder(const std::string& sampleId, const std::string& customerName, int quantity);
    std::vector<Order> getReservedOrders() const;
    std::vector<Order> getAllOrders() const;
    bool approveOrder(int orderId);   // CONFIRMED if stock sufficient, else PRODUCING
    bool rejectOrder(int orderId);

private:
    OrderRepository&  orderRepo_;
    SampleRepository& sampleRepo_;
    ProductionQueue&  productionQueue_;

    int calcActualQty(int shortage, double yield) const;
};
