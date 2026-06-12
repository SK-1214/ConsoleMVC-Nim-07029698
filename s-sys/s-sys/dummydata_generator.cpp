// dummydata_generator.cpp
// 반도체 시료(Sample) 더미 데이터 생성기 (독립 실행 참조 파일 — 프로젝트에 컴파일되지 않음)
// 사용법: 이 파일만 단독 컴파일 후 실행, 개수 입력 → dummydata/ 폴더에 JSON 파일 저장
// avgProductionTime 단위: 초 (300~10800초 / 5분~3시간)
//
// 컴파일 예시 (MSVC Developer Command Prompt):
//   cl /std:c++17 /utf-8 /EHsc dummydata_generator.cpp
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
#include <windows.h>

namespace fs = std::filesystem;

// ─── 반도체 생산라인 시료 이름 풀 ────────────────────────────────────────────

static const std::vector<std::string> MATERIAL_NAMES = {
    "실리콘 웨이퍼 8인치",
    "실리콘 웨이퍼 12인치",
    "실리콘 웨이퍼 6인치",
    "SOI 웨이퍼 12인치",
    "에피택셜 실리콘 웨이퍼 12인치",
    "GAA 공정 웨이퍼 12인치",
    "FinFET 공정 웨이퍼 12인치",
    "EUV 노광 웨이퍼 12인치",
    "3nm 공정 웨이퍼 12인치",
    "5nm 공정 웨이퍼 12인치",
    "7nm 공정 웨이퍼 12인치",
    "DRAM DDR5 웨이퍼 12인치",
    "LPDDR5 모바일 DRAM 웨이퍼 12인치",
    "NAND Flash 176단 웨이퍼 12인치",
    "NAND Flash 238단 웨이퍼 12인치",
    "HBM3 웨이퍼 12인치",
    "HBM3E 웨이퍼 12인치",
    "SiC 파워 웨이퍼 6인치",
    "SiC 파워 웨이퍼 8인치",
    "GaN-on-Si 파워 웨이퍼 8인치",
    "고전압 MOSFET 웨이퍼 8인치",
    "IGBT 파워 모듈 웨이퍼 8인치",
    "GaAs RF 웨이퍼 6인치",
    "InP 고속 소자 웨이퍼 4인치",
    "GaN RF 웨이퍼 6인치",
    "AlGaN/GaN HEMT 웨이퍼 6인치",
    "CIS 이미지센서 웨이퍼 8인치",
    "ToF 센서 웨이퍼 8인치",
    "InGaAs 적외선 센서 웨이퍼 4인치",
    "VCSEL 레이저 웨이퍼 6인치",
    "모바일 AP 웨이퍼 12인치",
    "AI 가속기 웨이퍼 12인치",
    "자동차용 MCU 웨이퍼 8인치",
    "BCD 아날로그 혼성신호 웨이퍼 8인치",
    "eFlash MCU 웨이퍼 8인치",
    "TSV 인터포저 웨이퍼 12인치",
    "Fan-out WLP 웨이퍼 12인치",
    "2.5D 칩렛 인터포저 웨이퍼 12인치",
    "OLED 드라이버 IC 웨이퍼 8인치",
    "AMOLED DDI 웨이퍼 8인치",
    "MEMS 압력센서 웨이퍼 8인치",
    "RF CMOS 웨이퍼 12인치",
    "BiCMOS SiGe 웨이퍼 8인치",
    "고저항 실리콘 RF 웨이퍼 12인치",
    "FD-SOI 28nm 웨이퍼 12인치",
    "질화갈륨 LED 웨이퍼 4인치",
    "마이크로 LED 웨이퍼 6인치",
    "SiPh 실리콘 포토닉스 웨이퍼 12인치",
    "양자점 QD-LED 웨이퍼 8인치",
    "Ga2O3 차세대 파워 웨이퍼 4인치"
};

// ─── 유틸리티 ─────────────────────────────────────────────────────────────────

static std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) return {};
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring wstr(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr.data(), wlen);
    return wstr;
}

static std::string makeId(int index) {
    std::ostringstream oss;
    oss << "S" << std::setw(3) << std::setfill('0') << index;
    return oss.str();
}

static std::string makeSafeFilename(const std::string& id, const std::string& name) {
    std::string safe = name;
    for (char& c : safe) {
        if (static_cast<unsigned char>(c) < 0x80 &&
            (c == ' ' || c == '/' || c == '\\' || c == ':' ||
             c == '*'  || c == '?' || c == '"'  || c == '<' ||
             c == '>'  || c == '|'))
            c = '_';
    }
    return id + "_" + safe + ".json";
}

// ─── 메인 ─────────────────────────────────────────────────────────────────────

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "========================================\n";
    std::cout << "  S-Semi 더미 데이터 생성기\n";
    std::cout << "========================================\n";
    std::cout << "생성할 시료 데이터 개수를 입력하세요: ";

    int count = 0;
    std::cin >> count;

    if (count <= 0) {
        std::cerr << "오류: 1 이상의 숫자를 입력해야 합니다.\n";
        return 1;
    }

    const fs::path outDir = fs::path(L"dummydata");
    fs::create_directories(outDir);

    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int>     distTime(300, 10800);
    std::uniform_real_distribution<double> distYield(0.50, 1.00);
    std::uniform_int_distribution<int>     distStock(0, 500);

    std::cout << "\n생성 중...\n";

    int generated = 0;
    for (int i = 1; i <= count; ++i) {
        std::string name = MATERIAL_NAMES[(i - 1) % MATERIAL_NAMES.size()];
        int cycle = (i - 1) / static_cast<int>(MATERIAL_NAMES.size());
        if (cycle > 0)
            name += "-v" + std::to_string(cycle + 1);

        std::string id      = makeId(i);
        int    time         = distTime(rng);
        double yieldVal     = std::round(distYield(rng) * 100.0) / 100.0;
        int    stock        = distStock(rng);

        std::string   filenameUtf8 = makeSafeFilename(id, name);
        fs::path      filePath     = outDir / fs::path(toWide(filenameUtf8));
        std::ofstream ofs(filePath.wstring().c_str());
        if (!ofs.is_open()) {
            std::cerr << "파일 열기 실패: " << filenameUtf8 << "\n";
            continue;
        }

        ofs << std::fixed << std::setprecision(2);
        ofs << "{\n";
        ofs << "  \"id\": \""  << id   << "\",\n";
        ofs << "  \"name\": \"" << name << "\",\n";
        ofs << "  \"avgProductionTime\": " << time << ",\n";
        ofs << "  \"yield\": " << yieldVal << ",\n";
        ofs << "  \"stock\": " << stock << "\n";
        ofs << "}\n";
        ofs.close();

        std::cout << "[" << std::setw(3) << i << "/" << count << "] "
                  << filenameUtf8 << "\n";
        ++generated;
    }

    std::cout << "\n완료: 총 " << generated << "개의 시료 데이터가 dummydata/ 폴더에 저장됐습니다.\n";
    return 0;
}
