#pragma once
#include <string>
#include <vector>
#include <optional>
#include "model/Sample.h"
#include "model/SampleRepository.h"

class SampleController {
public:
    explicit SampleController(SampleRepository& repo);

    bool registerSample(const std::string& id, const std::string& name,
                        int avgProductionTime, double yield);
    std::vector<Sample> getAllSamples() const;
    std::vector<Sample> searchByName(const std::string& name) const;
    std::optional<Sample> findById(const std::string& id) const;

private:
    SampleRepository& repo_;
};
