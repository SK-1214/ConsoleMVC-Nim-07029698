#include "view/ProductionView.h"
#include <iostream>
#include <iomanip>

void ProductionView::displayCurrentProduction(const std::optional<ProductionJob>& job) const {
    std::cout << "\n--- 생산 라인 현황 ---\n";
    if (!job) {
        std::cout << "  현재 생산 중인 작업 없음\n";
        return;
    }
    std::cout << "  [생산 중]\n";
    std::cout << "  주문 ID   : " << job->orderId   << '\n';
    std::cout << "  시료 ID   : " << job->sampleId  << '\n';
    std::cout << "  생산 수량 : " << job->actualQty << '\n';
    std::cout << "  총 시간   : " << job->totalTime << '\n';
}

void ProductionView::displayWaitingQueue(const std::vector<ProductionJob>& jobs) const {
    std::cout << "\n  [대기 큐] " << jobs.size() << "건\n";
    if (jobs.empty()) return;
    std::cout << std::left
              << std::setw(10) << "주문 ID"
              << std::setw(12) << "시료 ID"
              << std::setw(10) << "수량"
              << std::setw(10) << "시간"
              << '\n';
    std::cout << std::string(42, '-') << '\n';
    for (const auto& j : jobs) {
        std::cout << std::setw(10) << j.orderId
                  << std::setw(12) << j.sampleId
                  << std::setw(10) << j.actualQty
                  << std::setw(10) << j.totalTime
                  << '\n';
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
