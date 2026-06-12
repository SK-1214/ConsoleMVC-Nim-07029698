#include "controller/ShipmentController.h"

ShipmentController::ShipmentController(OrderRepository& orderRepo)
    : orderRepo_(orderRepo) {}

std::vector<Order> ShipmentController::getConfirmedOrders() const {
    return orderRepo_.getByStatus(OrderStatus::CONFIRMED);
}

bool ShipmentController::release(int orderId) {
    auto order = orderRepo_.findById(orderId);
    if (!order || order->status != OrderStatus::CONFIRMED) return false;
    return orderRepo_.updateStatus(orderId, OrderStatus::RELEASE);
}
