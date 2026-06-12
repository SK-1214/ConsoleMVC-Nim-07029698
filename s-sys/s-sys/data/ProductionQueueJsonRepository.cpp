#include "data/ProductionQueueJsonRepository.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <windows.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

static std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) return {};
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring ws(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, ws.data(), wlen);
    return ws;
}

static ProductionJob jobFromJson(const json& j) {
    ProductionJob job;
    job.orderId   = j.at("orderId").get<int>();
    job.sampleId  = j.at("sampleId").get<std::string>();
    job.actualQty = j.at("actualQty").get<int>();
    job.totalTime = j.at("totalTime").get<int>();
    return job;
}

static json jobToJson(const ProductionJob& job) {
    return json{
        {"orderId",   job.orderId},
        {"sampleId",  job.sampleId},
        {"actualQty", job.actualQty},
        {"totalTime", job.totalTime}
    };
}

// ─────────────────────────────────────────────────────────────────────────────

ProductionQueueJsonRepository::ProductionQueueJsonRepository(const std::string& dir)
    : dir_(dir) {}

std::string ProductionQueueJsonRepository::filePath() const {
    return dir_ + "/production_queue.json";
}

std::vector<ProductionJob> ProductionQueueJsonRepository::loadJobs() const {
    std::vector<ProductionJob> jobs;
    fs::path fp(toWide(filePath()));
    if (!fs::exists(fp)) return jobs;

    std::ifstream ifs(fp.wstring().c_str());
    if (!ifs.is_open()) return jobs;

    try {
        json root = json::parse(ifs);
        for (const auto& item : root.value("jobs", json::array()))
            jobs.push_back(jobFromJson(item));
    } catch (...) {}

    return jobs;
}

bool ProductionQueueJsonRepository::save(const std::vector<ProductionJob>& jobs) const {
    fs::create_directories(fs::path(toWide(dir_)));
    std::ofstream ofs(toWide(filePath()).c_str());
    if (!ofs.is_open()) return false;

    json root;
    root["jobs"] = json::array();
    for (const auto& job : jobs)
        root["jobs"].push_back(jobToJson(job));
    ofs << root.dump(2);
    return true;
}
