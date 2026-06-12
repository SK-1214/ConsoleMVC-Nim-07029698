#pragma once
#include <optional>
#include <vector>
#include "model/ProductionQueue.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"

class ProductionController {
public:
    ProductionController(ProductionQueue& queue, OrderRepository& orderRepo,
                         SampleRepository& sampleRepo);

    std::optional<ProductionJob> getCurrentProduction() const;
    std::vector<ProductionJob>   getWaitingJobs() const;
    bool completeCurrentProduction();  // finish front job: update stock + order status

private:
    ProductionQueue&  queue_;
    OrderRepository&  orderRepo_;
    SampleRepository& sampleRepo_;
};
