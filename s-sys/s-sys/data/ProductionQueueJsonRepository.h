#pragma once
#include <string>
#include <vector>
#include "model/ProductionJob.h"

/*
 * producedata/production_queue.json 단일 파일 기반 생산 큐 저장소
 * { "jobs": [...] }  — 인덱스 0이 현재 생산 중인 작업(front)
 */
class ProductionQueueJsonRepository {
public:
    explicit ProductionQueueJsonRepository(const std::string& dir = "producedata");

    std::vector<ProductionJob> loadJobs() const;
    bool                       save(const std::vector<ProductionJob>& jobs) const;

private:
    std::string dir_;
    std::string filePath() const;
};
