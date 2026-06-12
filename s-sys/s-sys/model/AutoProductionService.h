#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include "SampleRepository.h"

struct ProductionEvent {
    std::string sampleId;
    std::string sampleName;
    int         unitsProduced;
};

/*
 * 시스템 타이머 기반 자동 재고 생산 서비스
 *
 * tick() 를 호출할 때마다 마지막 호출 이후 경과한 실시간(초)을 계산하여
 * 각 시료의 avgProductionTime / yield 기준으로 재고를 자동 증가시킨다.
 *
 * 생산 주기:
 *   accumulated[id] += elapsed_sec
 *   cycles = floor(accumulated[id] / avgProductionTime)
 *   accumulated[id] -= cycles * avgProductionTime
 *   stock_delta = floor(cycles * yield)   (수율 적용)
 */
class AutoProductionService {
public:
    explicit AutoProductionService(SampleRepository& repo);

    // 경과 시간을 계산해 재고를 갱신하고, 생산된 시료 목록을 반환
    std::vector<ProductionEvent> tick();

private:
    SampleRepository&                       repo_;
    std::chrono::steady_clock::time_point   lastTick_;
    std::unordered_map<std::string, double> timeResiduals_;  // 미완료 누적 시간(초)
    std::unordered_map<std::string, double> yieldResiduals_; // 수율 미만 누적 생산량
};
