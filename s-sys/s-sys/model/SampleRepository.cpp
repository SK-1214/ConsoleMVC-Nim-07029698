#include "SampleRepository.h"

bool SampleRepository::add(const Sample& sample) {
    if (exists(sample.id)) return false;
    samples_[sample.id] = sample;
    return true;
}

bool SampleRepository::exists(const std::string& id) const {
    return samples_.contains(id);
}

std::optional<Sample> SampleRepository::findById(const std::string& id) const {
    auto it = samples_.find(id);
    if (it == samples_.end()) return std::nullopt;
    return it->second;
}

std::vector<Sample> SampleRepository::findByName(const std::string& name) const {
    std::vector<Sample> result;
    for (const auto& [id, s] : samples_) {
        if (s.name.find(name) != std::string::npos)
            result.push_back(s);
    }
    return result;
}

std::vector<Sample> SampleRepository::getAll() const {
    std::vector<Sample> result;
    result.reserve(samples_.size());
    for (const auto& [id, s] : samples_)
        result.push_back(s);
    return result;
}

bool SampleRepository::updateStock(const std::string& id, int delta) {
    auto it = samples_.find(id);
    if (it == samples_.end()) return false;
    it->second.stock += delta;
    if (it->second.stock < 0) it->second.stock = 0;
    return true;
}

int SampleRepository::getStock(const std::string& id) const {
    auto it = samples_.find(id);
    if (it == samples_.end()) return 0;
    return it->second.stock;
}
