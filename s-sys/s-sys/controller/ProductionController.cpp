#include "controller/ProductionController.h"

ProductionController::ProductionController(ProductionQueue& queue, OrderRepository& orderRepo,
                                            SampleRepository& sampleRepo)
    : queue_(queue), orderRepo_(orderRepo), sampleRepo_(sampleRepo) {}

std::optional<ProductionJob> ProductionController::getCurrentProduction() const {
    return queue_.front();
}

std::vector<ProductionJob> ProductionController::getWaitingJobs() const {
    return queue_.getWaiting();
}

bool ProductionController::completeCurrentProduction() {
    auto job = queue_.front();
    if (!job) return false;
    sampleRepo_.updateStock(job->sampleId, job->actualQty);
    orderRepo_.updateStatus(job->orderId, OrderStatus::CONFIRMED);
    queue_.dequeue();
    return true;
}
