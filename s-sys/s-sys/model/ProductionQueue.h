#pragma once
#include <string>
#include <queue>
#include <vector>
#include <optional>

struct ProductionJob {
    int         orderId;
    std::string sampleId;
    int         actualQty;   // ceil(shortage / (yield * 0.9))
    int         totalTime;   // avgProductionTime * actualQty
};

class ProductionQueue {
public:
    void enqueue(const ProductionJob& job);
    std::optional<ProductionJob> front() const;
    void dequeue();
    bool empty() const;
    size_t size() const;
    std::vector<ProductionJob> getWaiting() const;  // all jobs except the front

private:
    std::queue<ProductionJob> queue_;
};
