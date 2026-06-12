#include "view/OrderView.h"
#include <iostream>
#include <iomanip>

void OrderView::displayMenu() const {
    std::cout << "\n--- 주문 관리 ---\n";
    std::cout << " 1. 주문 접수 (예약)\n";
    std::cout << " 2. 주문 승인 / 거절\n";
    std::cout << " 0. 뒤로\n";
    std::cout << "선택: ";
}

OrderMenuChoice OrderView::getChoice() const {
    int input = -1;
    std::cin >> input;
    std::cin.ignore();
    switch (input) {
    case 1:  return OrderMenuChoice::PLACE;
    case 2:  return OrderMenuChoice::APPROVE_REJECT;
    default: return OrderMenuChoice::BACK;
    }
}

OrderInput OrderView::inputOrderData() const {
    OrderInput o;
    std::cout << "시료 ID: ";
    std::cin >> o.sampleId;
    std::cout << "고객명: ";
    std::cin >> o.customerName;
    std::cout << "주문 수량: ";
    std::cin >> o.quantity;
    std::cin.ignore();
    return o;
}

int OrderView::inputOrderId() const {
    int id = 0;
    std::cout << "주문 ID 입력: ";
    std::cin >> id;
    std::cin.ignore();
    return id;
}

ApproveRejectChoice OrderView::getApproveRejectChoice() const {
    std::cout << " 1. 승인\n 2. 거절\n 0. 취소\n선택: ";
    int input = -1;
    std::cin >> input;
    std::cin.ignore();
    switch (input) {
    case 1:  return ApproveRejectChoice::APPROVE;
    case 2:  return ApproveRejectChoice::REJECT;
    default: return ApproveRejectChoice::BACK;
    }
}

void OrderView::displayOrders(const std::vector<Order>& orders) const {
    if (orders.empty()) {
        std::cout << "  (접수된 주문 없음)\n";
        return;
    }
    std::cout << "\n"
              << std::left
              << std::setw(8)  << "ID"
              << std::setw(12) << "시료 ID"
              << std::setw(14) << "고객명"
              << std::setw(8)  << "수량"
              << std::setw(12) << "상태"
              << '\n';
    std::cout << std::string(54, '-') << '\n';
    for (const auto& o : orders) {
        std::cout << std::setw(8)  << o.id
                  << std::setw(12) << o.sampleId
                  << std::setw(14) << o.customerName
                  << std::setw(8)  << o.quantity
                  << std::setw(12) << orderStatusToString(o.status)
                  << '\n';
    }
}

void OrderView::displayResult(bool success, const std::string& msg) const {
    std::cout << (success ? "[OK] " : "[실패] ") << msg << '\n';
}
