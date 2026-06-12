#include "view/ProductionView.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

static int dispWidth(const std::string& s) {
    int w = 0; size_t i = 0;
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
    return s + std::string(std::max(0, col - dispWidth(s)), ' ');
}
static std::string padNumR(int n, int col) {
    std::string s = std::to_string(n);
    return std::string(std::max(0, col - (int)s.size()), ' ') + s;
}

// 초 → "HH:MM:SS" 문자열
static std::string toHMS(int sec) {
    if (sec < 0) sec = 0;
    int h = sec / 3600, m = (sec % 3600) / 60, s = sec % 60;
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    return buf;
}

void ProductionView::displayCurrentProduction(const std::optional<ProductionProgress>& progress) const {
    std::cout << "\n--- 생산 라인 현황 ---\n";
    if (!progress) {
        std::cout << "  현재 생산 중인 작업 없음\n";
        return;
    }
    const auto& job = progress->job;
    int pct = (job.totalTime > 0)
              ? std::min(100, static_cast<int>(progress->elapsedSec * 100 / job.totalTime))
              : 100;

    // 진행 바 (20칸)
    int filled = pct / 5;
    std::string bar = "[" + std::string(filled, '#') + std::string(20 - filled, '-') + "]";

    std::cout << "  [생산 중]\n";
    std::cout << "  주문 ID   : " << job.orderId                 << '\n';
    std::cout << "  시료 ID   : " << job.sampleId                << '\n';
    std::cout << "  생산 수량 : " << job.actualQty               << "개\n";
    std::cout << "  총 시간   : " << toHMS(job.totalTime)        << '\n';
    std::cout << "  경과 시간 : " << toHMS(progress->elapsedSec) << '\n';
    std::cout << "  잔여 시간 : " << toHMS(progress->remainingSec) << '\n';
    std::cout << "  진행률    : " << bar << " " << pct << "%\n";
}

void ProductionView::displayWaitingQueue(const std::vector<ProductionJob>& jobs) const {
    std::cout << "\n  [대기 큐] " << jobs.size() << "건\n";
    if (jobs.empty()) return;

    constexpr int W_OID  =  8;
    constexpr int W_SID  =  9;
    constexpr int W_QTY  =  7;
    constexpr int W_TIME = 10;
    constexpr int TOTAL  = W_OID + W_SID + W_QTY + W_TIME + 3;

    std::cout << padR("주문ID", W_OID) << " "
              << padR("시료ID", W_SID) << " "
              << padR("수량",   W_QTY) << " "
              << padR("총시간", W_TIME) << '\n';
    std::cout << std::string(TOTAL, '-') << '\n';

    for (const auto& j : jobs) {
        std::cout << padR(std::to_string(j.orderId), W_OID) << " "
                  << padR(j.sampleId,                W_SID) << " "
                  << padNumR(j.actualQty,            W_QTY) << " "
                  << padR(toHMS(j.totalTime),        W_TIME) << '\n';
    }
}

ProductionMenuChoice ProductionView::getChoice() const {
    std::cout << "\n 1. 현재 생산 완료 처리\n 0. 뒤로\n선택: ";
    int input = -1;
    std::cin >> input;
    std::cin.ignore();
    return (input == 1) ? ProductionMenuChoice::COMPLETE : ProductionMenuChoice::BACK;
}

void ProductionView::displayResult(bool success, const std::string& msg) const {
    std::cout << (success ? "[OK] " : "[실패] ") << msg << '\n';
}
