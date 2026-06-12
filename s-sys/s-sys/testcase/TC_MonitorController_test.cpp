#include <gtest/gtest.h>
#include <filesystem>
#include "controller/MonitorController.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"
#include "model/StockInfo.h"

namespace fs = std::filesystem;
static const std::string TC_MONITOR_CTRL_DIR = "test_monitor_ctrl_tmp";

class MonitorControllerTest : public ::testing::Test {
protected:
    OrderRepository   orderRepo;
    SampleRepository  sampleRepo{ TC_MONITOR_CTRL_DIR };
    MonitorController ctrl{ orderRepo, sampleRepo };

    void SetUp() override {
        fs::create_directories(TC_MONITOR_CTRL_DIR);
    }

    void TearDown() override {
        fs::remove_all(TC_MONITOR_CTRL_DIR);
    }

    void addSample(const std::string& id, int stock = 100) {
        Sample s{ id, "시료_" + id, 60, 0.9, stock };
        sampleRepo.add(s);
    }

    int addOrder(const std::string& sampleId, int qty, OrderStatus status) {
        Order o;
        o.sampleId     = sampleId;
        o.customerName = "고객";
        o.quantity     = qty;
        o.status       = OrderStatus::RESERVED;
        int id = orderRepo.add(o);
        if (status != OrderStatus::RESERVED)
            orderRepo.updateStatus(id, status);
        return id;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// getOrdersByStatus — 상태별 주문 조회
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(MonitorControllerTest, GetOrdersByStatus_Reserved_ReturnsMatchingOrders) {
    addSample("S001");
    addOrder("S001", 10, OrderStatus::RESERVED);
    addOrder("S001",  5, OrderStatus::CONFIRMED);

    auto result = ctrl.getOrdersByStatus(OrderStatus::RESERVED);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].status, OrderStatus::RESERVED);
}

TEST_F(MonitorControllerTest, GetOrdersByStatus_Producing_ReturnsMatchingOrders) {
    addSample("S001");
    addOrder("S001", 10, OrderStatus::PRODUCING);
    addOrder("S001", 10, OrderStatus::PRODUCING);
    addOrder("S001",  5, OrderStatus::CONFIRMED);

    auto result = ctrl.getOrdersByStatus(OrderStatus::PRODUCING);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(MonitorControllerTest, GetOrdersByStatus_Confirmed_ReturnsMatchingOrders) {
    addSample("S001");
    addOrder("S001", 10, OrderStatus::CONFIRMED);
    addOrder("S001", 10, OrderStatus::RESERVED);

    auto result = ctrl.getOrdersByStatus(OrderStatus::CONFIRMED);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].status, OrderStatus::CONFIRMED);
}

TEST_F(MonitorControllerTest, GetOrdersByStatus_Release_ReturnsMatchingOrders) {
    addSample("S001");
    addOrder("S001", 10, OrderStatus::RELEASE);
    addOrder("S001", 10, OrderStatus::RESERVED);

    auto result = ctrl.getOrdersByStatus(OrderStatus::RELEASE);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].status, OrderStatus::RELEASE);
}

TEST_F(MonitorControllerTest, GetOrdersByStatus_Rejected_ReturnsEmpty) {
    // PRD §4.5.1: REJECTED는 모니터링에서 제외
    addSample("S001");
    addOrder("S001", 10, OrderStatus::REJECTED);

    auto result = ctrl.getOrdersByStatus(OrderStatus::REJECTED);
    EXPECT_TRUE(result.empty());
}

TEST_F(MonitorControllerTest, GetOrdersByStatus_NoOrders_ReturnsEmpty) {
    EXPECT_TRUE(ctrl.getOrdersByStatus(OrderStatus::RESERVED).empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// getStockInfo — 재고 상태 판정
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(MonitorControllerTest, GetStockInfo_NoSamples_ReturnsEmpty) {
    EXPECT_TRUE(ctrl.getStockInfo().empty());
}

TEST_F(MonitorControllerTest, GetStockInfo_Depleted_StockIsZero) {
    addSample("S001", 0);
    auto result = ctrl.getStockInfo();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].stockStatus, StockStatus::DEPLETED);
}

TEST_F(MonitorControllerTest, GetStockInfo_Shortage_StockLessThanPending) {
    // stock=5, PRODUCING qty=20 → shortage
    addSample("S001", 5);
    addOrder("S001", 20, OrderStatus::PRODUCING);

    auto result = ctrl.getStockInfo();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].stockStatus, StockStatus::SHORTAGE);
    EXPECT_EQ(result[0].pendingQty,  20);
}

TEST_F(MonitorControllerTest, GetStockInfo_Surplus_StockSufficientForPending) {
    // stock=100, PRODUCING qty=30 → surplus
    addSample("S001", 100);
    addOrder("S001", 30, OrderStatus::PRODUCING);

    auto result = ctrl.getStockInfo();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].stockStatus, StockStatus::SURPLUS);
}

TEST_F(MonitorControllerTest, GetStockInfo_NoActiveOrders_Surplus) {
    addSample("S001", 50);
    auto result = ctrl.getStockInfo();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].stockStatus, StockStatus::SURPLUS);
    EXPECT_EQ(result[0].pendingQty,  0);
}

TEST_F(MonitorControllerTest, GetStockInfo_PendingQtyAccumulatesMultipleProducingOrders) {
    addSample("S001", 10);
    addOrder("S001", 15, OrderStatus::PRODUCING);
    addOrder("S001", 10, OrderStatus::PRODUCING);

    auto result = ctrl.getStockInfo();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].pendingQty, 25);
}

TEST_F(MonitorControllerTest, GetStockInfo_MultipleSamples_EachEvaluatedIndependently) {
    addSample("S001", 0);    // DEPLETED
    addSample("S002", 100);  // SURPLUS (no pending)
    addSample("S003", 5);    // SHORTAGE (pending 20)
    addOrder("S003", 20, OrderStatus::PRODUCING);

    auto result = ctrl.getStockInfo();
    ASSERT_EQ(result.size(), 3u);

    for (const auto& info : result) {
        if (info.sample.id == "S001")
            EXPECT_EQ(info.stockStatus, StockStatus::DEPLETED);
        else if (info.sample.id == "S002")
            EXPECT_EQ(info.stockStatus, StockStatus::SURPLUS);
        else if (info.sample.id == "S003")
            EXPECT_EQ(info.stockStatus, StockStatus::SHORTAGE);
    }
}
