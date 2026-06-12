#pragma once
#include <string>
#include <queue>
#include <vector>
#include <optional>
#include "model/ProductionJob.h"
#include "data/ProductionQueueJsonRepository.h"

class ProductionQueue {
public:
    explicit ProductionQueue(const std::string& dir = "producedata");

    void enqueue(const ProductionJob& job);
    std::optional<ProductionJob> front() const;
    void dequeue();
    bool empty() const;
    size_t size() const;
    std::vector<ProductionJob> getWaiting() const;  // all jobs except the front

private:
    ProductionQueueJsonRepository json_;
    std::queue<ProductionJob>     queue_;

    void persist() const;
};
