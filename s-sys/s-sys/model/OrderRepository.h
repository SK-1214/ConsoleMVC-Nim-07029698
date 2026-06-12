#pragma once
#include <vector>
#include <optional>
#include "Order.h"

class OrderRepository {
public:
    int add(const Order& order);                                // returns assigned id
    std::optional<Order> findById(int orderId) const;
    std::vector<Order> getAll() const;
    std::vector<Order> getByStatus(OrderStatus status) const;
    bool updateStatus(int orderId, OrderStatus status);

private:
    std::vector<Order> orders_;
    int nextId_ = 1;
};
