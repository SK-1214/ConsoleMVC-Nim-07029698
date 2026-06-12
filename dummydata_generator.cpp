// dummydata_generator.cpp
// 반도체 시료(Sample) 더미 데이터 생성기
// 사용법: 프로그램 실행 후 생성 개수 입력 → dummydata/ 폴더에 JSON 파일 저장
// avgProductionTime 단위: 초 (300~10800초 / 5분~3시간)
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

// ─── 반도체 생산라인 시료 이름 풀 ────────────────────────────────────────────

static const std::vector<std::string> MATERIAL_NAMES = {
    // 실리콘 웨이퍼 계열
    "실리콘 웨이퍼 8인치",
    "실리콘 웨이퍼 12인치",
    "실리콘 웨이퍼 6인치",
    "SOI 웨이퍼 12인치",
    "에피택셜 실리콘 웨이퍼 12인치",
    // 최신 공정 웨이퍼
    "GAA 공정 웨이퍼 12인치",
    "FinFET 공정 웨이퍼 12인치",
    "EUV 노광 웨이퍼 12인치",
    "3nm 공정 웨이퍼 12인치",
    "5nm 공정 웨이퍼 12인치",
    "7nm 공정 웨이퍼 12인치",
    // 메모리 계열
    "DRAM DDR5 웨이퍼 12인치",
    "LPDDR5 모바일 DRAM 웨이퍼 12인치",
    "NAND Flash 176단 웨이퍼 12인치",
    "NAND Flash 238단 웨이퍼 12인치",
    "HBM3 웨이퍼 12인치",
    "HBM3E 웨이퍼 12인치",
    // 파워 반도체 계열
    "SiC 파워 웨이퍼 6인치",
    "SiC 파워 웨이퍼 8인치",
    "GaN-on-Si 파워 웨이퍼 8인치",
    "고전압 MOSFET 웨이퍼 8인치",
    "IGBT 파워 모듈 웨이퍼 8인치",
    // 화합물 반도체 계열
    "GaAs RF 웨이퍼 6인치",
    "InP 고속 소자 웨이퍼 4인치",
    "GaN RF 웨이퍼 6인치",
    "AlGaN/GaN HEMT 웨이퍼 6인치",
    // 광반도체 계열
    "CIS 이미지센서 웨이퍼 8인치",
    "ToF 센서 웨이퍼 8인치",
    "InGaAs 적외선 센서 웨이퍼 4인치",
    "VCSEL 레이저 웨이퍼 6인치",
    // SoC / 로직 계열
    "모바일 AP 웨이퍼 12인치",
    "AI 가속기 웨이퍼 12인치",
    "자동차용 MCU 웨이퍼 8인치",
    "BCD 아날로그 혼성신호 웨이퍼 8인치",
    "eFlash MCU 웨이퍼 8인치",
    // 패키징 기반 계열
    "TSV 인터포저 웨이퍼 12인치",
    "Fan-out WLP 웨이퍼 12인치",
    "2.5D 칩렛 인터포저 웨이퍼 12인치",
    // 디스플레이 구동 계열
    "OLED 드라이버 IC 웨이퍼 8인치",
    "AMOLED DDI 웨이퍼 8인치",
    // 기타 특수 공정
    "MEMS 압력센서 웨이퍼 8인치",
    "RF CMOS 웨이퍼 12인치",
    "BiCMOS SiGe 웨이퍼 8인치",
    "고저항 실리콘 RF 웨이퍼 12인치",
    "FD-SOI 28nm 웨이퍼 12인치",
    "질화갈륨 LED 웨이퍼 4인치",
    "마이크로 LED 웨이퍼 6인치",
    "SiPh 실리콘 포토닉스 웨이퍼 12인치",
    "양자점 QD-LED 웨이퍼 8인치"
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
        if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*' ||
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
    std::uniform_int_distribution<int>    distTime(300, 10800); // 생산시간 300~10800초 (5분~3시간)
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
        int   time           = distTime(rng);          // 초 단위
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
