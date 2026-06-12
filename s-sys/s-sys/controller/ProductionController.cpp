#include "controller/ProductionController.h"
#include <algorithm>

ProductionController::ProductionController(ProductionQueue& queue, OrderRepository& orderRepo,
                                            SampleRepository& sampleRepo)
    : queue_(queue), orderRepo_(orderRepo), sampleRepo_(sampleRepo) {}

// TC 호환: ProductionJob 직접 반환
std::optional<ProductionJob> ProductionController::getCurrentProduction() const {
    return queue_.front();
}

// View 전용: 경과·잔여 시간 포함
std::optional<ProductionProgress> ProductionController::getCurrentProductionProgress() const {
    auto job = queue_.front();
    if (!job) return std::nullopt;

    if (!jobStartTime_)
        jobStartTime_ = std::chrono::steady_clock::now();

    int elapsed   = static_cast<int>(std::chrono::duration<double>(
                        std::chrono::steady_clock::now() - *jobStartTime_).count());
    int remaining = std::max(0, job->totalTime - elapsed);

    return ProductionProgress{ *job, elapsed, remaining };
}

std::vector<ProductionJob> ProductionController::getWaitingJobs() const {
    return queue_.getWaiting();
}

// totalTime 경과 시 자동 완료 → 완료된 job 반환
std::optional<ProductionJob> ProductionController::tickProduction() {
    auto job = queue_.front();
    if (!job) return std::nullopt;

    // 시작 시각 초기화
    if (!jobStartTime_)
        jobStartTime_ = std::chrono::steady_clock::now();

    double elapsed = std::chrono::duration<double>(
                         std::chrono::steady_clock::now() - *jobStartTime_).count();

    if (elapsed >= static_cast<double>(job->totalTime)) {
        ProductionJob completed = *job;
        finishJob(completed);
        return completed;
    }

    return std::nullopt;
}

bool ProductionController::completeCurrentProduction() {
    auto job = queue_.front();
    if (!job) return false;
    finishJob(*job);
    return true;
}

void ProductionController::finishJob(const ProductionJob& job) {
    sampleRepo_.updateStock(job.sampleId, job.actualQty);
    orderRepo_.updateStatus(job.orderId, OrderStatus::CONFIRMED);
    queue_.dequeue();
    jobStartTime_ = std::nullopt; // 다음 작업을 위해 초기화
}
