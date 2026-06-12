#include "view/SampleView.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <string>

// UTF-8 표시 너비: 한글/CJK = 2칸, ASCII = 1칸
static int dispWidth(const std::string& s) {
    int w = 0;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if      (c < 0x80) { w += 1; i += 1; }
        else if (c < 0xE0) { w += 2; i += 2; }
        else if (c < 0xF0) { w += 2; i += 3; }
        else               { w += 2; i += 4; }
    }
    return w;
}

static std::string padR(const std::string& s, int col) {
    int pad = std::max(0, col - dispWidth(s));
    return s + std::string(pad, ' ');
}

static std::string padNumR(int n, int col) {
    std::string s = std::to_string(n);
    int pad = std::max(0, col - static_cast<int>(s.size()));
    return std::string(pad, ' ') + s;
}

void SampleView::displayMenu() const {
    std::cout << "\n--- 시료 관리 ---\n";
    std::cout << " 1. 시료 등록\n";
    std::cout << " 2. 시료 목록 조회\n";
    std::cout << " 3. 시료 검색\n";
    std::cout << " 0. 뒤로\n";
    std::cout << "선택: ";
}

SampleMenuChoice SampleView::getChoice() const {
    int input = -1;
    std::cin >> input;
    std::cin.ignore();
    switch (input) {
    case 1:  return SampleMenuChoice::REGISTER;
    case 2:  return SampleMenuChoice::LIST;
    case 3:  return SampleMenuChoice::SEARCH;
    default: return SampleMenuChoice::BACK;
    }
}

Sample SampleView::inputSampleData() const {
    Sample s;
    std::cout << "시료 ID: ";
    std::cin >> s.id;
    std::cout << "이름: ";
    std::cin >> s.name;
    std::cout << "생산 시간(정수): ";
    std::cin >> s.avgProductionTime;
    std::cout << "수율 (0.0~1.0): ";
    std::cin >> s.yield;
    std::cin.ignore();
    return s;
}

std::string SampleView::inputSearchName() const {
    std::string name;
    std::cout << "검색할 이름: ";
    std::cin >> name;
    std::cin.ignore();
    return name;
}

void SampleView::displaySamples(const std::vector<Sample>& samples) const {
    if (samples.empty()) {
        std::cout << "  (등록된 시료 없음)\n";
        return;
    }

    constexpr int W_ID   =  8;   // "S001    "
    constexpr int W_NAME = 34;   // 최장 소재명 수용
    constexpr int W_TIME = 12;   // 생산시간(초)
    constexpr int W_YILD =  8;   // 수율
    constexpr int W_STK  =  6;   // 재고
    constexpr int TOTAL  = W_ID + W_NAME + W_TIME + W_YILD + W_STK + 4;

    std::cout << '\n'
              << padR("시료ID",   W_ID)   << " "
              << padR("이름",     W_NAME)  << " "
              << padR("생산시간", W_TIME)  << " "
              << padR("수율",     W_YILD)  << " "
              << padR("재고",     W_STK)
              << '\n';
    std::cout << std::string(TOTAL, '-') << '\n';

    for (const auto& s : samples) {
        // 수율: 소수점 2자리 고정 표현
        std::ostringstream yieldStr;
        yieldStr << std::fixed << std::setprecision(2) << s.yield;

        std::cout << padR(s.id,           W_ID)   << " "
                  << padR(s.name,         W_NAME)  << " "
                  << padNumR(s.avgProductionTime, W_TIME) << " "
                  << padR(yieldStr.str(), W_YILD)  << " "
                  << padNumR(s.stock,     W_STK)
                  << '\n';
    }
}

void SampleView::displayResult(bool success, const std::string& msg) const {
    std::cout << (success ? "[OK] " : "[실패] ") << msg << '\n';
}
