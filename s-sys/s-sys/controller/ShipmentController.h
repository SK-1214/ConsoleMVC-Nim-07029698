#pragma once
#include <vector>
#include "model/Order.h"
#include "model/OrderRepository.h"

class ShipmentController {
public:
    explicit ShipmentController(OrderRepository& orderRepo);

    std::vector<Order> getConfirmedOrders() const;
    bool release(int orderId);  // CONFIRMED → RELEASE

private:
    OrderRepository& orderRepo_;
};
