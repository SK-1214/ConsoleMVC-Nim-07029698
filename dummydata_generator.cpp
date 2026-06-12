// dummydata_generator.cpp
// 반도체 시료(Sample) 더미 데이터 생성기
// 사용법: 프로그램 실행 후 생성 개수 입력 → dummydata/ 폴더에 JSON 파일 저장
//
// 컴파일 예시 (MSVC):
//   cl /std:c++17 /EHsc dummydata_generator.cpp
// 컴파일 예시 (g++):
//   g++ -std=c++17 -o dummydata_generator dummydata_generator.cpp

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace fs = std::filesystem;

// ─── 반도체 소재 풀 ───────────────────────────────────────────────────────────

static const std::vector<std::string> MATERIAL_NAMES = {
    "AlGaN", "GaN", "SiC", "InP", "GaAs",
    "InGaAs", "Si", "Ge", "ZnO", "HgCdTe",
    "InGaN", "AlInN", "AlGaAs", "InGaP", "GaSb",
    "InAs", "InSb", "AlN", "CdTe", "CdS",
    "ZnS", "ZnSe", "GaP", "GaN-HV", "SiGe",
    "Diamond", "Ga2O3", "BN", "AlGaN-UV", "InAlGaN",
    "MoS2", "WSe2", "GaN-RF", "SiC-4H", "AlGaP",
    "InAlAs", "GaInSb", "CdZnTe", "PbTe", "BiTe",
    "GaN-LED", "AlInGaP", "InGaAsP", "GaInN", "AlGaInN",
    "SiC-6H", "GaAs-HB", "InP-DHB", "ZnMgO", "CuInSe"
};

// ─── 유틸리티 ─────────────────────────────────────────────────────────────────

static std::string makeId(int index) {
    std::ostringstream oss;
    oss << "S" << std::setw(3) << std::setfill('0') << index;
    return oss.str();
}

static std::string makeFilename(const std::string& id, const std::string& name) {
    // 파일명에 사용할 수 없는 문자 치환
    std::string safeName = name;
    for (char& c : safeName) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' ||
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            c = '_';
    }
    return id + "_" + safeName + ".json";
}

static std::string buildJson(const std::string& id,
                             const std::string& name,
                             int avgProductionTime,
                             double yield,
                             int stock) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n";
    oss << "  \"id\": \""   << id   << "\",\n";
    oss << "  \"name\": \"" << name << "\",\n";
    oss << "  \"avgProductionTime\": " << avgProductionTime << ",\n";
    oss << "  \"yield\": "  << yield << ",\n";
    oss << "  \"stock\": "  << stock << "\n";
    oss << "}\n";
    return oss.str();
}

// ─── 메인 ─────────────────────────────────────────────────────────────────────

int main() {
    int count = 0;
    std::cout << "생성할 더미 데이터 개수를 입력하세요: ";
    std::cin >> count;

    if (count <= 0) {
        std::cerr << "오류: 1 이상의 숫자를 입력해야 합니다.\n";
        return 1;
    }

    // dummydata 디렉터리 생성
    const fs::path outDir = "dummydata";
    fs::create_directories(outDir);

    // 난수 엔진 초기화
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int>    distTime(10, 90);   // 생산시간 10~90분
    std::uniform_real_distribution<double> distYield(0.50, 1.00); // 수율 0.50~1.00
    std::uniform_int_distribution<int>    distStock(0, 500);  // 재고 0~500

    int generated = 0;

    for (int i = 1; i <= count; ++i) {
        const std::string& matName = MATERIAL_NAMES[(i - 1) % MATERIAL_NAMES.size()];

        // 소재 풀을 순환하되 index가 겹치면 접미사(_v2, _v3 ...) 추가
        std::string name = matName;
        int cycle = (i - 1) / static_cast<int>(MATERIAL_NAMES.size());
        if (cycle > 0) {
            name += "-v" + std::to_string(cycle + 1);
        }

        std::string id       = makeId(i);
        int   time           = distTime(rng);
        double yieldVal      = std::round(distYield(rng) * 100.0) / 100.0;
        int   stock          = distStock(rng);

        fs::path filePath = outDir / makeFilename(id, name);
        std::ofstream ofs(filePath);
        if (!ofs.is_open()) {
            std::cerr << "파일 열기 실패: " << filePath << "\n";
            continue;
        }
        ofs << buildJson(id, name, time, yieldVal, stock);
        ofs.close();

        std::cout << "[" << std::setw(3) << i << "/" << count << "] "
                  << filePath.string() << " 생성 완료\n";
        ++generated;
    }

    std::cout << "\n총 " << generated << "개의 더미 데이터가 "
              << outDir.string() << "/ 폴더에 저장됐습니다.\n";
    return 0;
}
