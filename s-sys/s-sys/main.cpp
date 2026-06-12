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
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

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

// UTF-8 std::string → std::wstring (Windows 파일 경로용)
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

// 파일명 금지 문자 → '_' 치환 (ASCII 범위만 처리, 한글 멀티바이트는 그대로 유지)
static std::string makeSafeFilename(const std::string& id, const std::string& name) {
    std::string safe = name;
    for (char& c : safe) {
        // ASCII 범위만 검사 (한글 멀티바이트 바이트는 상위 비트가 1이므로 건드리지 않음)
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
    std::uniform_int_distribution<int>     distTime(300, 10800); // 초 (5분~3시간)
    std::uniform_real_distribution<double> distYield(0.50, 1.00);
    std::uniform_int_distribution<int>     distStock(0, 500);

    std::cout << "\n생성 중...\n";

    int generated = 0;
    for (int i = 1; i <= count; ++i) {
        std::string name = MATERIAL_NAMES[(i - 1) % MATERIAL_NAMES.size()];
        int cycle = (i - 1) / static_cast<int>(MATERIAL_NAMES.size());
        if (cycle > 0)
            name += "-v" + std::to_string(cycle + 1);

        std::string id  = makeId(i);
        int    time     = distTime(rng);
        double yieldVal = std::round(distYield(rng) * 100.0) / 100.0;
        int    stock    = distStock(rng);

        json j;
        j["id"]                 = id;
        j["name"]               = name;
        j["avgProductionTime"]  = time;
        j["yield"]              = yieldVal;
        j["stock"]              = stock;

        // 한글 포함 경로는 wstring으로 변환하여 MSVC 파일스트림에 전달
        std::string   filenameUtf8 = makeSafeFilename(id, name);
        fs::path      filePath     = outDir / fs::path(toWide(filenameUtf8));
        std::ofstream ofs(filePath.wstring());
        if (!ofs.is_open()) {
            std::cerr << "파일 열기 실패: " << filenameUtf8 << "\n";
            continue;
        }
        ofs << j.dump(2);
        ofs.close();

        std::cout << "[" << std::setw(3) << i << "/" << count << "] "
                  << filenameUtf8 << "\n";
        ++generated;
    }

    std::cout << "\n완료: 총 " << generated << "개의 시료 데이터가 "
              << "dummydata" << " 폴더에 저장됐습니다.\n";
    return 0;
}
