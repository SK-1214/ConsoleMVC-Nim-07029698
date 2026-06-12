#include "data/SampleJsonRepository.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>

using json = nlohmann::json;
namespace fs = std::filesystem;

// ─── JSON 변환 헬퍼 ───────────────────────────────────────────────────────────

static Sample fromJson(const json& j) {
    return Sample{
        j.at("id").get<std::string>(),
        j.at("name").get<std::string>(),
        j.at("avgProductionTime").get<int>(),
        j.at("yield").get<double>(),
        j.value("stock", 0)
    };
}

static json toJson(const Sample& s) {
    return json{
        {"id",                 s.id},
        {"name",               s.name},
        {"avgProductionTime",  s.avgProductionTime},
        {"yield",              s.yield},
        {"stock",              s.stock}
    };
}

// ─── 생성자 ──────────────────────────────────────────────────────────────────

SampleJsonRepository::SampleJsonRepository(const std::string& filePath)
    : filePath_(filePath) {
    load();
}

// ─── private 헬퍼 ────────────────────────────────────────────────────────────

void SampleJsonRepository::load() {
    samples_.clear();
    if (!fs::exists(filePath_)) return;

    std::ifstream ifs(filePath_);
    if (!ifs.is_open()) return;

    try {
        json root = json::parse(ifs);
        for (const auto& item : root.value("samples", json::array()))
            samples_.push_back(fromJson(item));
    } catch (const json::exception&) {
        // 파일이 손상된 경우 빈 상태로 시작
        samples_.clear();
    }
}

int SampleJsonRepository::indexOf(const std::string& id) const {
    for (int i = 0; i < static_cast<int>(samples_.size()); ++i)
        if (samples_[i].id == id) return i;
    return -1;
}

// ─── Create ──────────────────────────────────────────────────────────────────

bool SampleJsonRepository::add(const Sample& sample) {
    if (exists(sample.id)) return false;
    if (sample.id.empty() || sample.name.empty() ||
        sample.avgProductionTime <= 0 ||
        sample.yield <= 0.0 || sample.yield > 1.0)
        return false;

    samples_.push_back(sample);
    return save();
}

// ─── Read ────────────────────────────────────────────────────────────────────

std::optional<Sample> SampleJsonRepository::findById(const std::string& id) const {
    int idx = indexOf(id);
    if (idx < 0) return std::nullopt;
    return samples_[idx];
}

std::vector<Sample> SampleJsonRepository::findByName(const std::string& name) const {
    std::vector<Sample> result;
    for (const auto& s : samples_)
        if (s.name.find(name) != std::string::npos)
            result.push_back(s);
    return result;
}

std::vector<Sample> SampleJsonRepository::getAll() const {
    return samples_;
}

bool SampleJsonRepository::exists(const std::string& id) const {
    return indexOf(id) >= 0;
}

// ─── Update ──────────────────────────────────────────────────────────────────

bool SampleJsonRepository::updateStock(const std::string& id, int delta) {
    int idx = indexOf(id);
    if (idx < 0) return false;
    samples_[idx].stock += delta;
    if (samples_[idx].stock < 0) samples_[idx].stock = 0;
    return save();
}

bool SampleJsonRepository::update(const Sample& sample) {
    int idx = indexOf(sample.id);
    if (idx < 0) return false;
    samples_[idx] = sample;
    return save();
}

// ─── Delete ──────────────────────────────────────────────────────────────────

bool SampleJsonRepository::remove(const std::string& id) {
    int idx = indexOf(id);
    if (idx < 0) return false;
    samples_.erase(samples_.begin() + idx);
    return save();
}

// ─── 영속화 ──────────────────────────────────────────────────────────────────

bool SampleJsonRepository::save() const {
    // 디렉터리가 없으면 생성
    fs::path dir = fs::path(filePath_).parent_path();
    if (!dir.empty()) fs::create_directories(dir);

    std::ofstream ofs(filePath_);
    if (!ofs.is_open()) return false;

    json root;
    root["samples"] = json::array();
    for (const auto& s : samples_)
        root["samples"].push_back(toJson(s));

    ofs << root.dump(2);
    return true;
}
