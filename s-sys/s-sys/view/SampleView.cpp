#include "view/SampleView.h"
#include <iostream>
#include <iomanip>

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
    std::cout << "평균 생산 시간 (정수): ";
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
    std::cout << "\n"
              << std::left
              << std::setw(10) << "ID"
              << std::setw(16) << "이름"
              << std::setw(12) << "생산시간"
              << std::setw(8)  << "수율"
              << std::setw(8)  << "재고"
              << '\n';
    std::cout << std::string(54, '-') << '\n';
    for (const auto& s : samples) {
        std::cout << std::setw(10) << s.id
                  << std::setw(16) << s.name
                  << std::setw(12) << s.avgProductionTime
                  << std::setw(8)  << s.yield
                  << std::setw(8)  << s.stock
                  << '\n';
    }
}

void SampleView::displayResult(bool success, const std::string& msg) const {
    std::cout << (success ? "[OK] " : "[실패] ") << msg << '\n';
}
