#include "controller/OrderController.h"
#include <cmath>

OrderController::OrderController(OrderRepository& orderRepo, SampleRepository& sampleRepo,
                                  ProductionQueue& productionQueue)
    : orderRepo_(orderRepo), sampleRepo_(sampleRepo), productionQueue_(productionQueue) {}

int OrderController::placeOrder(const std::string& sampleId,
                                 const std::string& customerName, int quantity) {
    if (!sampleRepo_.exists(sampleId) || customerName.empty() || quantity <= 0)
        return -1;
    Order o;
    o.sampleId      = sampleId;
    o.customerName  = customerName;
    o.quantity      = quantity;
    o.status        = OrderStatus::RESERVED;
    return orderRepo_.add(o);
}

std::vector<Order> OrderController::getReservedOrders() const {
    return orderRepo_.getByStatus(OrderStatus::RESERVED);
}

bool OrderController::approveOrder(int orderId) {
    auto order = orderRepo_.findById(orderId);
    if (!order || order->status != OrderStatus::RESERVED) return false;

    auto sample = sampleRepo_.findById(order->sampleId);
    if (!sample) return false;

    int stock = sampleRepo_.getStock(order->sampleId);
    if (stock >= order->quantity) {
        sampleRepo_.updateStock(order->sampleId, -order->quantity);
        orderRepo_.updateStatus(orderId, OrderStatus::CONFIRMED);
    } else {
        int shortage   = order->quantity - stock;
        int actualQty  = calcActualQty(shortage, sample->yield);
        ProductionJob job{ orderId, order->sampleId, actualQty,
                           sample->avgProductionTime * actualQty };
        productionQueue_.enqueue(job);
        orderRepo_.updateStatus(orderId, OrderStatus::PRODUCING);
    }
    return true;
}

bool OrderController::rejectOrder(int orderId) {
    auto order = orderRepo_.findById(orderId);
    if (!order || order->status != OrderStatus::RESERVED) return false;
    return orderRepo_.updateStatus(orderId, OrderStatus::REJECTED);
}

int OrderController::calcActualQty(int shortage, double yield) const {
    return static_cast<int>(std::ceil(static_cast<double>(shortage) / (yield * 0.9)));
}
