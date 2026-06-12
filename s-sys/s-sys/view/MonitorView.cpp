#include "view/MonitorView.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
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

// 오른쪽 공백 패딩 (표시 너비 기준)
static std::string padR(const std::string& s, int col) {
    int pad = std::max(0, col - dispWidth(s));
    return s + std::string(pad, ' ');
}

// 숫자를 오른쪽 정렬 문자열로 변환
static std::string padNumR(int n, int col) {
    std::string s = std::to_string(n);
    int pad = std::max(0, col - static_cast<int>(s.size()));
    return std::string(pad, ' ') + s;
}

// ─── 주문 목록 출력 ──────────────────────────────────────────────────────────

void MonitorView::displayOrdersByStatus(const std::vector<Order>& orders,
                                         OrderStatus status) const {
    // 열 너비 (표시 칸 수)
    constexpr int W_ID   =  7;   // "주문ID " → 숫자 최대 4자리 + 여유
    constexpr int W_SID  =  9;   // "S001   "
    constexpr int W_NAME = 22;   // 한글 고객명 최대 ~10자
    constexpr int W_QTY  =  7;   // 수량
    constexpr int W_STAT = 12;   // "PRODUCING"

    constexpr int TOTAL  = W_ID + W_SID + W_NAME + W_QTY + W_STAT + 4; // 4 = 구분자

    std::cout << "\n[" << orderStatusToString(status) << " 주문 목록]  "
              << orders.size() << "건\n";

    if (orders.empty()) return;

    // 헤더
    std::cout << padR("주문ID",  W_ID)  << " "
              << padR("시료ID",  W_SID) << " "
              << padR("고객명",  W_NAME) << " "
              << padR("수량",    W_QTY)  << " "
              << padR("상태",    W_STAT)
              << '\n';
    std::cout << std::string(TOTAL, '-') << '\n';

    // 데이터 행
    for (const auto& o : orders) {
        std::cout << padR(std::to_string(o.id),  W_ID)  << " "
                  << padR(o.sampleId,             W_SID) << " "
                  << padR(o.customerName,          W_NAME) << " "
                  << padNumR(o.quantity,           W_QTY)  << " "
                  << padR(orderStatusToString(o.status), W_STAT)
                  << '\n';
    }
}

// ─── 재고 현황 출력 ──────────────────────────────────────────────────────────

void MonitorView::displayStockInfo(const std::vector<SampleStockInfo>& info) const {
    std::cout << "\n[재고 현황]\n";
    if (info.empty()) {
        std::cout << "  (등록된 시료 없음)\n";
        return;
    }

    // 열 너비 (표시 칸 수)
    constexpr int W_ID    =  8;   // "S001    "
    constexpr int W_NAME  = 34;   // "GaN-on-Si 파워 웨이퍼 8인치" 등 최장 소재명
    constexpr int W_STOCK =  7;   // 재고 수량
    constexpr int W_PEND  =  9;   // 처리 대기
    constexpr int W_STAT  =  6;   // "여유"/"부족"/"고갈"

    constexpr int TOTAL   = W_ID + W_NAME + W_STOCK + W_PEND + W_STAT + 4;

    // 헤더
    std::cout << padR("시료ID",   W_ID)   << " "
              << padR("이름",     W_NAME)  << " "
              << padR("재고",     W_STOCK) << " "
              << padR("처리대기", W_PEND)  << " "
              << padR("상태",     W_STAT)
              << '\n';
    std::cout << std::string(TOTAL, '-') << '\n';

    // 데이터 행
    for (const auto& si : info) {
        std::cout << padR(si.sample.id,                   W_ID)   << " "
                  << padR(si.sample.name,                 W_NAME)  << " "
                  << padNumR(si.sample.stock,             W_STOCK) << " "
                  << padNumR(si.pendingQty,               W_PEND)  << " "
                  << padR(stockStatusToString(si.stockStatus), W_STAT)
                  << '\n';
    }
}
