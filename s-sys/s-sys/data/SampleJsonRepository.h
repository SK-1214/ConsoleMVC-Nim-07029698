#pragma once
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include "model/Sample.h"

/*
 * JSON 파일 기반 Sample CRUD 저장소
 *
 * [디렉터리 모드] path가 ".json"으로 끝나지 않을 때 (기본값: "sampledata")
 *   - sampledata/{id}_{name}.json 형태로 시료마다 개별 파일 저장
 *   - add/update/remove 시 해당 파일만 생성·수정·삭제
 *
 * [단일 파일 모드] path가 ".json"으로 끝날 때 (테스트 호환용)
 *   - 기존 { "samples": [...] } 단일 JSON 파일 방식 유지
 */
class SampleJsonRepository {
public:
    explicit SampleJsonRepository(const std::string& path = "sampledata");

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

    // 현재 메모리 상태를 파일(들)에 전체 저장
    bool save() const;

private:
    std::string         path_;
    bool                dirMode_;
    std::vector<Sample> samples_;

    void load();
    int  indexOf(const std::string& id) const;

    // 디렉터리 모드 전용
    bool saveOne(const Sample& s) const;
    bool removeFile(const Sample& s) const;
    std::filesystem::path sampleFilePath(const Sample& s) const;
    static std::string    makeSafeFilename(const Sample& s);
};
