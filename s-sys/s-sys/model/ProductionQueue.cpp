#include "ProductionQueue.h"

void ProductionQueue::enqueue(const ProductionJob& job) {
    queue_.push(job);
}

std::optional<ProductionJob> ProductionQueue::front() const {
    if (queue_.empty()) return std::nullopt;
    return queue_.front();
}

void ProductionQueue::dequeue() {
    if (!queue_.empty()) queue_.pop();
}

bool ProductionQueue::empty() const {
    return queue_.empty();
}

size_t ProductionQueue::size() const {
    return queue_.size();
}

std::vector<ProductionJob> ProductionQueue::getWaiting() const {
    std::vector<ProductionJob> result;
    auto copy = queue_;
    if (!copy.empty()) copy.pop();  // skip front (currently in production)
    while (!copy.empty()) {
        result.push_back(copy.front());
        copy.pop();
    }
    return result;
}
