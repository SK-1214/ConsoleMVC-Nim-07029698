#pragma once
#include <optional>
#include <vector>
#include <string>
#include "model/ProductionQueue.h"
#include "controller/ProductionController.h"

enum class ProductionMenuChoice { COMPLETE = 1, BACK = 0 };

class ProductionView {
public:
    void displayCurrentProduction(const std::optional<ProductionProgress>& progress) const;
    void displayWaitingQueue(const std::vector<ProductionJob>& jobs) const;
    ProductionMenuChoice getChoice() const;
    void displayResult(bool success, const std::string& msg) const;
};
