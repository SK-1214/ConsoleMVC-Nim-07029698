#include <gtest/gtest.h>
#include <filesystem>
#include <cmath>
#include "controller/OrderController.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"
#include "model/ProductionQueue.h"

namespace fs = std::filesystem;
static const std::string TC_ORDER_CTRL_DIR = "test_order_ctrl_tmp";

class OrderControllerTest : public ::testing::Test {
protected:
    OrderRepository  orderRepo;
    SampleRepository sampleRepo{ TC_ORDER_CTRL_DIR };
    ProductionQueue  queue;
    OrderController  ctrl{ orderRepo, sampleRepo, queue };

    void SetUp() override {
        fs::create_directories(TC_ORDER_CTRL_DIR);
    }

    void TearDown() override {
        fs::remove_all(TC_ORDER_CTRL_DIR);
    }

    void addSample(const std::string& id, int stock = 100,
                   double yield = 0.9, int avgTime = 60) {
        Sample s{ id, "시료_" + id, avgTime, yield, stock };
        sampleRepo.add(s);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// placeOrder
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderControllerTest, PlaceOrder_ValidSample_ReturnsPositiveId) {
    addSample("S001");
    EXPECT_GT(ctrl.placeOrder("S001", "고객A", 10), 0);
}

TEST_F(OrderControllerTest, PlaceOrder_ValidSample_StatusIsReserved) {
    addSample("S001");
    int id = ctrl.placeOrder("S001", "고객A", 10);
    auto order = orderRepo.findById(id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::RESERVED);
}

TEST_F(OrderControllerTest, PlaceOrder_ValidSample_FieldsStoredCorrectly) {
    addSample("S001");
    int id = ctrl.placeOrder("S001", "홍길동", 25);
    auto order = orderRepo.findById(id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->sampleId,     "S001");
    EXPECT_EQ(order->customerName, "홍길동");
    EXPECT_EQ(order->quantity,     25);
}

TEST_F(OrderControllerTest, PlaceOrder_UnregisteredSample_ReturnsMinusOne) {
    EXPECT_EQ(ctrl.placeOrder("NONE", "고객A", 10), -1);
}

TEST_F(OrderControllerTest, PlaceOrder_UnregisteredSample_NoOrderCreated) {
    ctrl.placeOrder("NONE", "고객A", 10);
    EXPECT_TRUE(orderRepo.getAll().empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// getReservedOrders
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderControllerTest, GetReservedOrders_NoOrders_ReturnsEmpty) {
    EXPECT_TRUE(ctrl.getReservedOrders().empty());
}

TEST_F(OrderControllerTest, GetReservedOrders_MixedStatuses_OnlyReservedReturned) {
    addSample("S001");
    int id1 = ctrl.placeOrder("S001", "고객A", 10);
    int id2 = ctrl.placeOrder("S001", "고객B", 5);
    orderRepo.updateStatus(id2, OrderStatus::CONFIRMED);

    auto reserved = ctrl.getReservedOrders();
    ASSERT_EQ(reserved.size(), 1u);
    EXPECT_EQ(reserved[0].id, id1);
}

// ═══════════════════════════════════════════════════════════════════════════
// approveOrder — 재고 충분 → CONFIRMED
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderControllerTest, ApproveOrder_SufficientStock_ReturnsTrue) {
    addSample("S001", 100);
    int id = ctrl.placeOrder("S001", "고객A", 50);
    EXPECT_TRUE(ctrl.approveOrder(id));
}

TEST_F(OrderControllerTest, ApproveOrder_SufficientStock_StatusBecomesConfirmed) {
    addSample("S001", 100);
    int id = ctrl.placeOrder("S001", "고객A", 50);
    ctrl.approveOrder(id);
    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::CONFIRMED);
}

TEST_F(OrderControllerTest, ApproveOrder_SufficientStock_StockDecremented) {
    addSample("S001", 100);
    int id = ctrl.placeOrder("S001", "고객A", 30);
    ctrl.approveOrder(id);
    EXPECT_EQ(sampleRepo.getStock("S001"), 70);
}

TEST_F(OrderControllerTest, ApproveOrder_SufficientStock_QueueRemainsEmpty) {
    addSample("S001", 100);
    int id = ctrl.placeOrder("S001", "고객A", 10);
    ctrl.approveOrder(id);
    EXPECT_TRUE(queue.empty());
}

TEST_F(OrderControllerTest, ApproveOrder_ExactStock_StatusConfirmed) {
    addSample("S001", 10);
    int id = ctrl.placeOrder("S001", "고객A", 10);
    ctrl.approveOrder(id);
    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::CONFIRMED);
}

// ═══════════════════════════════════════════════════════════════════════════
// approveOrder — 재고 부족 → PRODUCING + 생산 큐 등록
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderControllerTest, ApproveOrder_InsufficientStock_StatusBecomesProducing) {
    addSample("S001", 5);
    int id = ctrl.placeOrder("S001", "고객A", 20);
    ctrl.approveOrder(id);
    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::PRODUCING);
}

TEST_F(OrderControllerTest, ApproveOrder_InsufficientStock_JobEnqueuedWithOrderId) {
    addSample("S001", 5);
    int id = ctrl.placeOrder("S001", "고객A", 20);
    ctrl.approveOrder(id);
    ASSERT_FALSE(queue.empty());
    EXPECT_EQ(queue.front()->orderId, id);
}

TEST_F(OrderControllerTest, ApproveOrder_InsufficientStock_ActualQtyFormula) {
    // stock=5, qty=20 → shortage=15, yield=0.9
    // actualQty = ceil(15 / (0.9 * 0.9)) = ceil(18.52) = 19
    addSample("S001", 5, 0.9, 60);
    int id = ctrl.placeOrder("S001", "고객A", 20);
    ctrl.approveOrder(id);
    int expected = (int)std::ceil(15.0 / (0.9 * 0.9));
    EXPECT_EQ(queue.front()->actualQty, expected);
}

TEST_F(OrderControllerTest, ApproveOrder_InsufficientStock_TotalTimeFormula) {
    // avgProductionTime=60, shortage=15, yield=0.9
    // actualQty = ceil(15/0.81) = 19, totalTime = 60*19 = 1140
    addSample("S001", 5, 0.9, 60);
    int id = ctrl.placeOrder("S001", "고객A", 20);
    ctrl.approveOrder(id);
    int actualQty = (int)std::ceil(15.0 / (0.9 * 0.9));
    EXPECT_EQ(queue.front()->totalTime, 60 * actualQty);
}

TEST_F(OrderControllerTest, ApproveOrder_ZeroStock_FullQuantityIsShortage) {
    // stock=0, qty=10, yield=0.8
    // actualQty = ceil(10 / (0.8 * 0.9)) = ceil(13.89) = 14
    addSample("S001", 0, 0.8, 30);
    int id = ctrl.placeOrder("S001", "고객A", 10);
    ctrl.approveOrder(id);
    int expected = (int)std::ceil(10.0 / (0.8 * 0.9));
    EXPECT_EQ(queue.front()->actualQty, expected);
}

TEST_F(OrderControllerTest, ApproveOrder_InsufficientStock_SampleIdInJob) {
    addSample("S001", 5);
    int id = ctrl.placeOrder("S001", "고객A", 20);
    ctrl.approveOrder(id);
    EXPECT_EQ(queue.front()->sampleId, "S001");
}

// ═══════════════════════════════════════════════════════════════════════════
// rejectOrder
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderControllerTest, RejectOrder_ReservedOrder_ReturnsTrue) {
    addSample("S001");
    int id = ctrl.placeOrder("S001", "고객A", 10);
    EXPECT_TRUE(ctrl.rejectOrder(id));
}

TEST_F(OrderControllerTest, RejectOrder_ReservedOrder_StatusBecomesRejected) {
    addSample("S001");
    int id = ctrl.placeOrder("S001", "고객A", 10);
    ctrl.rejectOrder(id);
    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::REJECTED);
}

TEST_F(OrderControllerTest, RejectOrder_NonExistingId_ReturnsFalse) {
    EXPECT_FALSE(ctrl.rejectOrder(9999));
}
