#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include "Sample.h"

class SampleRepository {
public:
    bool add(const Sample& sample);
    bool exists(const std::string& id) const;
    std::optional<Sample> findById(const std::string& id) const;
    std::vector<Sample> findByName(const std::string& name) const;
    std::vector<Sample> getAll() const;
    bool updateStock(const std::string& id, int delta);
    int  getStock(const std::string& id) const;

private:
    std::unordered_map<std::string, Sample> samples_;
};
