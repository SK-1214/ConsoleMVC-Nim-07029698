#include "OrderRepository.h"

int OrderRepository::add(const Order& order) {
    Order o  = order;
    o.id     = nextId_++;
    orders_.push_back(o);
    return o.id;
}

std::optional<Order> OrderRepository::findById(int orderId) const {
    for (const auto& o : orders_)
        if (o.id == orderId) return o;
    return std::nullopt;
}

std::vector<Order> OrderRepository::getAll() const {
    return orders_;
}

std::vector<Order> OrderRepository::getByStatus(OrderStatus status) const {
    std::vector<Order> result;
    for (const auto& o : orders_)
        if (o.status == status) result.push_back(o);
    return result;
}

bool OrderRepository::updateStatus(int orderId, OrderStatus status) {
    for (auto& o : orders_) {
        if (o.id == orderId) {
            o.status = status;
            return true;
        }
    }
    return false;
}
