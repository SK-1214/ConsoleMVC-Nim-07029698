#include "data/SampleJsonRepository.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <algorithm>
#include <windows.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

// ─── Windows UTF-8 경로 헬퍼 ──────────────────────────────────────────────────

static std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) return {};
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring ws(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, ws.data(), wlen);
    return ws;
}

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
        {"id",                s.id},
        {"name",              s.name},
        {"avgProductionTime", s.avgProductionTime},
        {"yield",             s.yield},
        {"stock",             s.stock}
    };
}

// ─── 생성자 ──────────────────────────────────────────────────────────────────

SampleJsonRepository::SampleJsonRepository(const std::string& path)
    : path_(path)
    , dirMode_(path.size() < 5 ||
               path.substr(path.size() - 5) != ".json") {
    load();
}

// ─── private 헬퍼 ────────────────────────────────────────────────────────────

std::string SampleJsonRepository::makeSafeFilename(const Sample& s) {
    std::string safe = s.name;
    for (char& c : safe) {
        if (static_cast<unsigned char>(c) < 0x80 &&
            (c == ' ' || c == '/' || c == '\\' || c == ':' ||
             c == '*'  || c == '?' || c == '"'  || c == '<' ||
             c == '>'  || c == '|'))
            c = '_';
    }
    return s.id + "_" + safe + ".json";
}

fs::path SampleJsonRepository::sampleFilePath(const Sample& s) const {
    return fs::path(toWide(path_)) / fs::path(toWide(makeSafeFilename(s)));
}

void SampleJsonRepository::load() {
    samples_.clear();

    if (dirMode_) {
        // ── 디렉터리 모드: 폴더 내 모든 .json 파일 읽기 ──
        fs::path dir(toWide(path_));
        if (!fs::exists(dir) || !fs::is_directory(dir)) return;

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() == L".json")
                files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());

        for (const auto& fp : files) {
            std::ifstream ifs(fp.wstring().c_str());
            if (!ifs.is_open()) continue;
            try {
                json j = json::parse(ifs);
                samples_.push_back(fromJson(j));
            } catch (...) {}
        }
    } else {
        // ── 단일 파일 모드 (테스트 호환) ──
        if (!fs::exists(toWide(path_))) return;
        std::ifstream ifs(toWide(path_).c_str());
        if (!ifs.is_open()) return;
        try {
            json root = json::parse(ifs);
            for (const auto& item : root.value("samples", json::array()))
                samples_.push_back(fromJson(item));
        } catch (...) {
            samples_.clear();
        }
    }
}

int SampleJsonRepository::indexOf(const std::string& id) const {
    for (int i = 0; i < static_cast<int>(samples_.size()); ++i)
        if (samples_[i].id == id) return i;
    return -1;
}

bool SampleJsonRepository::saveOne(const Sample& s) const {
    fs::create_directories(fs::path(toWide(path_)));
    std::ofstream ofs(sampleFilePath(s).wstring().c_str());
    if (!ofs.is_open()) return false;
    ofs << toJson(s).dump(2);
    return true;
}

bool SampleJsonRepository::removeFile(const Sample& s) const {
    fs::path fp = sampleFilePath(s);
    if (fs::exists(fp)) {
        fs::remove(fp);
        return true;
    }
    // 이름 변경 등으로 파일명이 달라진 경우 ID 접두어로 탐색
    fs::path dir(toWide(path_));
    if (!fs::exists(dir)) return false;
    for (const auto& entry : fs::directory_iterator(dir)) {
        std::string fname = entry.path().filename().string();
        if (fname.size() > s.id.size() &&
            fname.substr(0, s.id.size() + 1) == s.id + "_") {
            fs::remove(entry.path());
            return true;
        }
    }
    return false;
}

// ─── Create ──────────────────────────────────────────────────────────────────

bool SampleJsonRepository::add(const Sample& sample) {
    if (exists(sample.id)) return false;
    if (sample.id.empty() || sample.name.empty() ||
        sample.avgProductionTime <= 0 ||
        sample.yield <= 0.0 || sample.yield > 1.0)
        return false;

    samples_.push_back(sample);
    return dirMode_ ? saveOne(sample) : save();
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
    return dirMode_ ? saveOne(samples_[idx]) : save();
}

bool SampleJsonRepository::update(const Sample& sample) {
    int idx = indexOf(sample.id);
    if (idx < 0) return false;
    if (dirMode_ && samples_[idx].name != sample.name)
        removeFile(samples_[idx]);  // 이름 변경 시 구 파일 삭제
    samples_[idx] = sample;
    return dirMode_ ? saveOne(sample) : save();
}

// ─── Delete ──────────────────────────────────────────────────────────────────

bool SampleJsonRepository::remove(const std::string& id) {
    int idx = indexOf(id);
    if (idx < 0) return false;
    if (dirMode_) removeFile(samples_[idx]);
    samples_.erase(samples_.begin() + idx);
    return dirMode_ ? true : save();
}

// ─── 영속화 (전체) ────────────────────────────────────────────────────────────

bool SampleJsonRepository::save() const {
    if (dirMode_) {
        for (const auto& s : samples_)
            if (!saveOne(s)) return false;
        return true;
    }

    // 단일 파일 모드
    fs::path dir = fs::path(toWide(path_)).parent_path();
    if (!dir.empty()) fs::create_directories(dir);

    std::ofstream ofs(toWide(path_).c_str());
    if (!ofs.is_open()) return false;

    json root;
    root["samples"] = json::array();
    for (const auto& s : samples_)
        root["samples"].push_back(toJson(s));
    ofs << root.dump(2);
    return true;
}
