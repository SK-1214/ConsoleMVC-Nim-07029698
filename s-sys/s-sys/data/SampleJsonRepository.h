#pragma once
#include <string>
#include <vector>
#include <optional>
#include "model/Sample.h"

/*
 * JSON 파일 기반 Sample CRUD 저장소
 *
 * 파일 경로: data/samples.json (실행 파일 기준 상대 경로)
 *
 * JSON 스키마:
 * {
 *   "samples": [
 *     {
 *       "id":                 "S001",
 *       "name":               "AlGaN",
 *       "avgProductionTime":  30,
 *       "yield":              0.9,
 *       "stock":              100
 *     }
 *   ]
 * }
 */
class SampleJsonRepository {
public:
    explicit SampleJsonRepository(const std::string& filePath = "data/samples.json");

    // Create
    bool add(const Sample& sample);

    // Read
    std::optional<Sample> findById(const std::string& id) const;
    std::vector<Sample>   findByName(const std::string& name) const;
    std::vector<Sample>   getAll() const;
    bool                  exists(const std::string& id) const;

    // Update
    bool updateStock(const std::string& id, int delta);
    bool update(const Sample& sample);

    // Delete
    bool remove(const std::string& id);

    // 현재 메모리 상태를 JSON 파일에 저장
    bool save() const;

private:
    std::string         filePath_;
    std::vector<Sample> samples_;

    void load();
    int  indexOf(const std::string& id) const;
};
