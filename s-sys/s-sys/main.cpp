#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <windows.h>
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct SampleData {
    std::string id;
    std::string name;
    int         avgProductionTime = 0;
    double      yield             = 0.0;
    int         stock             = 0;
};

// UTF-8 std::string → std::wstring
static std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) return {};
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring ws(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, ws.data(), wlen);
    return ws;
}

// sampledata/ 폴더의 모든 .json 파일을 읽어 SampleData 목록 반환
static std::vector<SampleData> loadSamples(const fs::path& dir) {
    std::vector<SampleData> list;

    if (!fs::exists(dir) || !fs::is_directory(dir)) return list;

    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == L".json")
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());

    for (const auto& path : files) {
        std::ifstream ifs(path.wstring().c_str());
        if (!ifs.is_open()) continue;

        try {
            json j = json::parse(ifs);
            SampleData s;
            s.id                 = j.value("id",                 "");
            s.name               = j.value("name",               "");
            s.avgProductionTime  = j.value("avgProductionTime",  0);
            s.yield              = j.value("yield",              0.0);
            s.stock              = j.value("stock",              0);
            list.push_back(s);
        } catch (...) {
            std::cerr << "파싱 실패: " << path.filename() << "\n";
        }
    }
    return list;
}

static void printTable(const std::vector<SampleData>& samples) {
    if (samples.empty()) {
        std::cout << "  조회된 시료 데이터가 없습니다.\n";
        return;
    }

    const std::string sep(82, '-');
    std::cout << sep << "\n";
    std::cout << std::left
              << std::setw(8)  << "ID"
              << std::setw(36) << "시료명"
              << std::setw(14) << "생산시간(초)"
              << std::setw(10) << "수율"
              << std::setw(8)  << "재고"
              << "\n";
    std::cout << sep << "\n";

    for (const auto& s : samples) {
        std::cout << std::left
                  << std::setw(8)  << s.id
                  << std::setw(36) << s.name
                  << std::setw(14) << s.avgProductionTime
                  << std::setw(10) << std::fixed << std::setprecision(2) << s.yield
                  << std::setw(8)  << s.stock
                  << "\n";
    }
    std::cout << sep << "\n";
    std::cout << "  총 " << samples.size() << "개의 시료 데이터\n";
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const fs::path sampleDir = fs::path(L"sampledata");

    std::cout << "========================================\n";
    std::cout << "  S-Semi 샘플 데이터 모니터\n";
    std::cout << "========================================\n\n";

    std::cout << "sampledata/ 폴더에서 시료 데이터를 불러오는 중...\n\n";

    auto samples = loadSamples(sampleDir);
    printTable(samples);

    std::cout << "\n아무 키나 누르면 종료합니다...";
    std::cin.get();
    return 0;
}
