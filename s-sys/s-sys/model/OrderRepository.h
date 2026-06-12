#pragma once
#include <vector>
#include <optional>
#include <string>
#include "Order.h"
#include "data/OrderJsonRepository.h"

class OrderRepository {
public:
    explicit OrderRepository(const std::string& dir = "");

    int add(const Order& order);
    std::optional<Order> findById(int orderId) const;
    std::vector<Order> getAll() const;
    std::vector<Order> getByStatus(OrderStatus status) const;
    bool updateStatus(int orderId, OrderStatus status);

private:
    OrderJsonRepository json_;
};
