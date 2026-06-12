#include "OrderRepository.h"

OrderRepository::OrderRepository(const std::string& dir) : json_(dir) {}

int OrderRepository::add(const Order& order) {
    return json_.add(order);
}

std::optional<Order> OrderRepository::findById(int orderId) const {
    return json_.findById(orderId);
}

std::vector<Order> OrderRepository::getAll() const {
    return json_.getAll();
}

std::vector<Order> OrderRepository::getByStatus(OrderStatus status) const {
    return json_.getByStatus(status);
}

bool OrderRepository::updateStatus(int orderId, OrderStatus status) {
    return json_.updateStatus(orderId, status);
}
