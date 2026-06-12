#include "view/MonitorView.h"
#include <iostream>
#include <iomanip>

void MonitorView::displayOrdersByStatus(const std::vector<Order>& orders,
                                         OrderStatus status) const {
    std::cout << "\n[" << orderStatusToString(status) << " 주문 목록] "
              << orders.size() << "건\n";
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

void MonitorView::displayStockInfo(const std::vector<SampleStockInfo>& info) const {
    std::cout << "\n[재고 현황]\n";
    if (info.empty()) {
        std::cout << "  (등록된 시료 없음)\n";
        return;
    }
    std::cout << std::left
              << std::setw(10) << "시료 ID"
              << std::setw(16) << "이름"
              << std::setw(8)  << "재고"
              << std::setw(12) << "처리 대기"
              << std::setw(8)  << "상태"
              << '\n';
    std::cout << std::string(54, '-') << '\n';
    for (const auto& si : info) {
        std::cout << std::setw(10) << si.sample.id
                  << std::setw(16) << si.sample.name
                  << std::setw(8)  << si.sample.stock
                  << std::setw(12) << si.pendingQty
                  << std::setw(8)  << stockStatusToString(si.stockStatus)
                  << '\n';
    }
}
