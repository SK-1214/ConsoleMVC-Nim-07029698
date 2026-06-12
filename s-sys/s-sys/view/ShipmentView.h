#pragma once
#include <vector>
#include <string>
#include "model/Order.h"

class ShipmentView {
public:
    void displayConfirmedOrders(const std::vector<Order>& orders) const;
    int  inputOrderId() const;
    void displayResult(bool success, const std::string& msg) const;
};
