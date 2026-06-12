#include "controller/SampleController.h"

SampleController::SampleController(SampleRepository& repo) : repo_(repo) {}

bool SampleController::registerSample(const std::string& id, const std::string& name,
                                       int avgProductionTime, double yield) {
    if (id.empty() || name.empty() || avgProductionTime <= 0 || yield <= 0.0 || yield > 1.0)
        return false;
    Sample s{ id, name, avgProductionTime, yield, 0 };
    return repo_.add(s);
}

std::vector<Sample> SampleController::getAllSamples() const {
    return repo_.getAll();
}

std::vector<Sample> SampleController::searchByName(const std::string& name) const {
    return repo_.findByName(name);
}

std::optional<Sample> SampleController::findById(const std::string& id) const {
    return repo_.findById(id);
}
