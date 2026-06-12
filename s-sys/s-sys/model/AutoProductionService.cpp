#include "model/AutoProductionService.h"
#include <cmath>

AutoProductionService::AutoProductionService(SampleRepository& repo)
    : repo_(repo)
    , lastTick_(std::chrono::steady_clock::now())
{}

std::vector<ProductionEvent> AutoProductionService::tick() {
    auto   now     = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - lastTick_).count();
    lastTick_ = now;

    std::vector<ProductionEvent> events;

    for (const auto& s : repo_.getAll()) {
        if (s.avgProductionTime <= 0) continue;

        // ① 경과 시간 누적 → 완료된 생산 사이클 수 계산
        timeResiduals_[s.id] += elapsed;
        int cycles = static_cast<int>(timeResiduals_[s.id] / s.avgProductionTime);
        if (cycles <= 0) continue;
        timeResiduals_[s.id] -= cycles * s.avgProductionTime;

        // ② avgProductionTime 1사이클 = 1개 생산 (yield 미적용)
        int delta = cycles;

        repo_.updateStock(s.id, delta);
        events.push_back({ s.id, s.name, delta });
    }

    return events;
}
