#include "view/MainView.h"
#include <iostream>

void MainView::displayMenu() const {
    std::cout << "\n========================================\n";
    std::cout << "   S-Semi 반도체 시료 생산 주문 관리 시스템\n";
    std::cout << "========================================\n";
    std::cout << " 1. 시료 관리\n";
    std::cout << " 2. 주문 관리 (접수 / 승인 / 거절)\n";
    std::cout << " 3. 모니터링\n";
    std::cout << " 4. 출고 처리\n";
    std::cout << " 5. 생산 라인\n";
    std::cout << " 0. 종료\n";
    std::cout << "----------------------------------------\n";
    std::cout << "선택: ";
}

MainMenuChoice MainView::getChoice() const {
    int input = -1;
    std::cin >> input;
    std::cin.ignore();
    switch (input) {
    case 1:  return MainMenuChoice::SAMPLE_MANAGEMENT;
    case 2:  return MainMenuChoice::ORDER_MANAGEMENT;
    case 3:  return MainMenuChoice::MONITORING;
    case 4:  return MainMenuChoice::SHIPMENT;
    case 5:  return MainMenuChoice::PRODUCTION_LINE;
    default: return MainMenuChoice::EXIT;
    }
}

void MainView::displayMessage(const std::string& msg) const {
    std::cout << msg << '\n';
}
