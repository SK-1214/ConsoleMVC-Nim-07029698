#include <gtest/gtest.h>
#include <filesystem>
#include "controller/ProductionController.h"
#include "model/ProductionQueue.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"

namespace fs = std::filesystem;
static const std::string TC_PROD_CTRL_DIR = "test_prod_ctrl_tmp";

class ProductionControllerTest : public ::testing::Test {
protected:
    ProductionQueue      queue;
    OrderRepository      orderRepo;
    SampleRepository     sampleRepo{ TC_PROD_CTRL_DIR };
    ProductionController ctrl{ queue, orderRepo, sampleRepo };

    void SetUp() override {
        fs::create_directories(TC_PROD_CTRL_DIR);
    }

    void TearDown() override {
        fs::remove_all(TC_PROD_CTRL_DIR);
    }

    void addSample(const std::string& id, int stock = 0) {
        Sample s{ id, "시료_" + id, 60, 0.9, stock };
        sampleRepo.add(s);
    }

    int addProducingOrder(const std::string& sampleId, int qty = 10) {
        Order o;
        o.sampleId     = sampleId;
        o.customerName = "고객";
        o.quantity     = qty;
        o.status       = OrderStatus::RESERVED;
        int id = orderRepo.add(o);
        orderRepo.updateStatus(id, OrderStatus::PRODUCING);
        return id;
    }

    void enqueue(int orderId, const std::string& sampleId,
                 int actualQty = 12, int totalTime = 720) {
        queue.enqueue(ProductionJob{ orderId, sampleId, actualQty, totalTime });
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// getCurrentProduction
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ProductionControllerTest, GetCurrentProduction_EmptyQueue_ReturnsNullopt) {
    EXPECT_FALSE(ctrl.getCurrentProduction().has_value());
}

TEST_F(ProductionControllerTest, GetCurrentProduction_JobInQueue_ReturnsFrontJob) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    enqueue(id, "S001", 12, 720);

    auto job = ctrl.getCurrentProduction();
    ASSERT_TRUE(job.has_value());
    EXPECT_EQ(job->orderId,   id);
    EXPECT_EQ(job->sampleId,  "S001");
    EXPECT_EQ(job->actualQty, 12);
    EXPECT_EQ(job->totalTime, 720);
}

// ═══════════════════════════════════════════════════════════════════════════
// getWaitingJobs
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ProductionControllerTest, GetWaitingJobs_EmptyQueue_ReturnsEmpty) {
    EXPECT_TRUE(ctrl.getWaitingJobs().empty());
}

TEST_F(ProductionControllerTest, GetWaitingJobs_OnlyOneJob_ReturnsEmpty) {
    addSample("S001");
    int id = addProducingOrder("S001");
    enqueue(id, "S001");
    EXPECT_TRUE(ctrl.getWaitingJobs().empty());
}

TEST_F(ProductionControllerTest, GetWaitingJobs_TwoJobs_ReturnsTailOnly) {
    addSample("S001");
    int id1 = addProducingOrder("S001", 10);
    int id2 = addProducingOrder("S001",  5);
    enqueue(id1, "S001", 12, 720);
    enqueue(id2, "S001",  6, 360);

    auto waiting = ctrl.getWaitingJobs();
    ASSERT_EQ(waiting.size(), 1u);
    EXPECT_EQ(waiting[0].orderId, id2);
}

// ═══════════════════════════════════════════════════════════════════════════
// completeCurrentProduction
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ProductionControllerTest, CompleteCurrentProduction_EmptyQueue_ReturnsFalse) {
    EXPECT_FALSE(ctrl.completeCurrentProduction());
}

TEST_F(ProductionControllerTest, CompleteCurrentProduction_ValidJob_ReturnsTrue) {
    addSample("S001", 0);
    int id = addProducingOrder("S001", 10);
    enqueue(id, "S001", 12, 720);
    EXPECT_TRUE(ctrl.completeCurrentProduction());
}

TEST_F(ProductionControllerTest, CompleteCurrentProduction_OrderStatusBecomesConfirmed) {
    addSample("S001", 0);
    int id = addProducingOrder("S001", 10);
    enqueue(id, "S001", 12, 720);

    ctrl.completeCurrentProduction();

    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::CONFIRMED);
}

TEST_F(ProductionControllerTest, CompleteCurrentProduction_StockIncreasedByActualQty) {
    addSample("S001", 5);
    int id = addProducingOrder("S001", 10);
    enqueue(id, "S001", 12, 720);

    ctrl.completeCurrentProduction();

    EXPECT_EQ(sampleRepo.getStock("S001"), 5 + 12);
}

TEST_F(ProductionControllerTest, CompleteCurrentProduction_JobRemovedFromQueue) {
    addSample("S001", 0);
    int id = addProducingOrder("S001", 10);
    enqueue(id, "S001", 12, 720);

    ctrl.completeCurrentProduction();

    EXPECT_TRUE(queue.empty());
}

TEST_F(ProductionControllerTest, CompleteCurrentProduction_FIFO_AdvancesToNextJob) {
    addSample("S001", 0);
    int id1 = addProducingOrder("S001", 10);
    int id2 = addProducingOrder("S001",  5);
    enqueue(id1, "S001", 12, 720);
    enqueue(id2, "S001",  6, 360);

    ctrl.completeCurrentProduction();

    EXPECT_EQ(queue.size(), 1u);
    ASSERT_TRUE(ctrl.getCurrentProduction().has_value());
    EXPECT_EQ(ctrl.getCurrentProduction()->orderId, id2);
}

TEST_F(ProductionControllerTest, CompleteCurrentProduction_TwoJobs_BothComplete) {
    addSample("S001", 0);
    int id1 = addProducingOrder("S001", 10);
    int id2 = addProducingOrder("S001",  5);
    enqueue(id1, "S001", 12, 720);
    enqueue(id2, "S001",  6, 360);

    ctrl.completeCurrentProduction();
    ctrl.completeCurrentProduction();

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(orderRepo.findById(id1)->status, OrderStatus::CONFIRMED);
    EXPECT_EQ(orderRepo.findById(id2)->status, OrderStatus::CONFIRMED);
}
