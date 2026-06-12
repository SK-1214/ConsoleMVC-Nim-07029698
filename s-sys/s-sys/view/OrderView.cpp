#include "view/OrderView.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
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
    int pad = std::max(0, col - dispWidth(s));
    return s + std::string(pad, ' ');
}
static std::string padNumR(int n, int col) {
    std::string s = std::to_string(n);
    int pad = std::max(0, col - static_cast<int>(s.size()));
    return std::string(pad, ' ') + s;
}

void OrderView::displayMenu() const {
    std::cout << "\n--- 주문 관리 ---\n";
    std::cout << " 1. 주문 접수 (예약)\n";
    std::cout << " 2. 주문 승인 / 거절\n";
    std::cout << " 3. 주문 현황\n";
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
    case 3:  return OrderMenuChoice::STATUS;
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

static void printOrderTable(const std::vector<Order>& orders, const std::string& emptyMsg) {
    if (orders.empty()) {
        std::cout << "  (" << emptyMsg << ")\n";
        return;
    }

    constexpr int W_ID   =  7;
    constexpr int W_SID  =  9;
    constexpr int W_NAME = 22;
    constexpr int W_QTY  =  7;
    constexpr int W_STAT = 12;
    constexpr int TOTAL  = W_ID + W_SID + W_NAME + W_QTY + W_STAT + 4;

    std::cout << '\n'
              << padR("주문ID",  W_ID)   << " "
              << padR("시료ID",  W_SID)  << " "
              << padR("고객명",  W_NAME) << " "
              << padR("수량",    W_QTY)  << " "
              << padR("상태",    W_STAT)
              << '\n';
    std::cout << std::string(TOTAL, '-') << '\n';

    for (const auto& o : orders) {
        std::cout << padR(std::to_string(o.id),          W_ID)   << " "
                  << padR(o.sampleId,                    W_SID)  << " "
                  << padR(o.customerName,                W_NAME) << " "
                  << padNumR(o.quantity,                 W_QTY)  << " "
                  << padR(orderStatusToString(o.status), W_STAT)
                  << '\n';
    }
}

void OrderView::displayOrders(const std::vector<Order>& orders) const {
    printOrderTable(orders, "접수된 주문 없음");
}

void OrderView::displayAllOrders(const std::vector<Order>& orders) const {
    std::cout << "\n[주문 현황]  전체 " << orders.size() << "건";
    printOrderTable(orders, "등록된 주문 없음");
}

void OrderView::displayResult(bool success, const std::string& msg) const {
    std::cout << (success ? "[OK] " : "[실패] ") << msg << '\n';
}
