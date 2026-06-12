#pragma once
#include <chrono>
#include <optional>
#include <vector>
#include "model/ProductionJob.h"
#include "model/ProductionQueue.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"

// 현재 생산 중인 작업의 진행 상황
struct ProductionProgress {
    ProductionJob job;
    int elapsedSec;    // 경과 시간(초)
    int remainingSec;  // 잔여 시간(초)
};

class ProductionController {
public:
    ProductionController(ProductionQueue& queue, OrderRepository& orderRepo,
                         SampleRepository& sampleRepo);

    // 기존 인터페이스 유지 (TC 호환)
    std::optional<ProductionJob>      getCurrentProduction() const;
    std::vector<ProductionJob>        getWaitingJobs() const;

    // 경과·잔여 시간 포함 진행 상황 (View 전용)
    std::optional<ProductionProgress> getCurrentProductionProgress() const;

    // 타이머 tick: totalTime 경과 시 자동 완료 → 완료된 job 반환
    std::optional<ProductionJob> tickProduction();

    // 수동 완료 (테스트·강제 처리용)
    bool completeCurrentProduction();

private:
    ProductionQueue&  queue_;
    OrderRepository&  orderRepo_;
    SampleRepository& sampleRepo_;

    mutable std::optional<std::chrono::steady_clock::time_point> jobStartTime_;

    void finishJob(const ProductionJob& job);
};
