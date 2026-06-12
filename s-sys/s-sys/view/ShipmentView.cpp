#include "view/ShipmentView.h"
#include <iostream>
#include <iomanip>

void ShipmentView::displayConfirmedOrders(const std::vector<Order>& orders) const {
    std::cout << "\n--- 출고 처리 ---\n";
    std::cout << "[출고 대기 주문 (CONFIRMED)] " << orders.size() << "건\n";
    if (orders.empty()) return;
    std::cout << std::left
              << std::setw(8)  << "ID"
              << std::setw(12) << "시료 ID"
              << std::setw(14) << "고객명"
              << std::setw(8)  << "수량"
              << '\n';
    std::cout << std::string(42, '-') << '\n';
    for (const auto& o : orders) {
        std::cout << std::setw(8)  << o.id
                  << std::setw(12) << o.sampleId
                  << std::setw(14) << o.customerName
                  << std::setw(8)  << o.quantity
                  << '\n';
    }
}

int ShipmentView::inputOrderId() const {
    int id = 0;
    std::cout << "출고할 주문 ID (0: 취소): ";
    std::cin >> id;
    std::cin.ignore();
    return id;
}

void ShipmentView::displayResult(bool success, const std::string& msg) const {
    std::cout << (success ? "[OK] " : "[실패] ") << msg << '\n';
}
