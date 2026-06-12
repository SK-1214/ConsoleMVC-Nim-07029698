#include <gtest/gtest.h>
#include "controller/ShipmentController.h"
#include "model/OrderRepository.h"

class ShipmentControllerTest : public ::testing::Test {
protected:
    OrderRepository    orderRepo;
    ShipmentController ctrl{ orderRepo };

    int addOrder(OrderStatus status) {
        Order o;
        o.sampleId     = "S001";
        o.customerName = "고객";
        o.quantity     = 10;
        o.status       = OrderStatus::RESERVED;
        int id = orderRepo.add(o);
        if (status != OrderStatus::RESERVED)
            orderRepo.updateStatus(id, status);
        return id;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// getConfirmedOrders
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ShipmentControllerTest, GetConfirmedOrders_NoOrders_ReturnsEmpty) {
    EXPECT_TRUE(ctrl.getConfirmedOrders().empty());
}

TEST_F(ShipmentControllerTest, GetConfirmedOrders_OnlyConfirmedReturned) {
    addOrder(OrderStatus::RESERVED);
    addOrder(OrderStatus::PRODUCING);
    int confId = addOrder(OrderStatus::CONFIRMED);

    auto result = ctrl.getConfirmedOrders();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].id,     confId);
    EXPECT_EQ(result[0].status, OrderStatus::CONFIRMED);
}

TEST_F(ShipmentControllerTest, GetConfirmedOrders_MultipleConfirmed_AllReturned) {
    addOrder(OrderStatus::CONFIRMED);
    addOrder(OrderStatus::CONFIRMED);
    EXPECT_EQ(ctrl.getConfirmedOrders().size(), 2u);
}

// ═══════════════════════════════════════════════════════════════════════════
// release
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ShipmentControllerTest, Release_ConfirmedOrder_ReturnsTrue) {
    int id = addOrder(OrderStatus::CONFIRMED);
    EXPECT_TRUE(ctrl.release(id));
}

TEST_F(ShipmentControllerTest, Release_ConfirmedOrder_StatusBecomesRelease) {
    int id = addOrder(OrderStatus::CONFIRMED);
    ctrl.release(id);
    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::RELEASE);
}

TEST_F(ShipmentControllerTest, Release_ConfirmedOrder_NoLongerInConfirmedList) {
    int id = addOrder(OrderStatus::CONFIRMED);
    ctrl.release(id);
    EXPECT_TRUE(ctrl.getConfirmedOrders().empty());
}

TEST_F(ShipmentControllerTest, Release_ReservedOrder_ReturnsFalse) {
    int id = addOrder(OrderStatus::RESERVED);
    EXPECT_FALSE(ctrl.release(id));
}

TEST_F(ShipmentControllerTest, Release_ProducingOrder_ReturnsFalse) {
    int id = addOrder(OrderStatus::PRODUCING);
    EXPECT_FALSE(ctrl.release(id));
}

TEST_F(ShipmentControllerTest, Release_RejectedOrder_ReturnsFalse) {
    int id = addOrder(OrderStatus::REJECTED);
    EXPECT_FALSE(ctrl.release(id));
}

TEST_F(ShipmentControllerTest, Release_AlreadyReleasedOrder_ReturnsFalse) {
    int id = addOrder(OrderStatus::RELEASE);
    EXPECT_FALSE(ctrl.release(id));
}

TEST_F(ShipmentControllerTest, Release_NonExistingId_ReturnsFalse) {
    EXPECT_FALSE(ctrl.release(9999));
}

TEST_F(ShipmentControllerTest, Release_NonConfirmedOrder_StatusUnchanged) {
    int id = addOrder(OrderStatus::RESERVED);
    ctrl.release(id);
    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::RESERVED);
}
