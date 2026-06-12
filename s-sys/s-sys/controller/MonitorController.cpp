#include "controller/MonitorController.h"

MonitorController::MonitorController(OrderRepository& orderRepo, SampleRepository& sampleRepo)
    : orderRepo_(orderRepo), sampleRepo_(sampleRepo) {}

std::vector<Order> MonitorController::getOrdersByStatus(OrderStatus status) const {
    // PRD §4.5.1: REJECTED는 모니터링 대상에서 제외
    if (status == OrderStatus::REJECTED) return {};
    return orderRepo_.getByStatus(status);
}

std::vector<SampleStockInfo> MonitorController::getStockInfo() const {
    std::vector<SampleStockInfo> result;
    for (const auto& sample : sampleRepo_.getAll()) {
        int stock = sample.stock;

        int pending = 0;
        for (const auto& o : orderRepo_.getByStatus(OrderStatus::PRODUCING))
            if (o.sampleId == sample.id) pending += o.quantity;

        StockStatus status;
        if (stock == 0)            status = StockStatus::DEPLETED;
        else if (stock < pending)  status = StockStatus::SHORTAGE;
        else                       status = StockStatus::SURPLUS;

        result.push_back({ sample, pending, status });
    }
    return result;
}
