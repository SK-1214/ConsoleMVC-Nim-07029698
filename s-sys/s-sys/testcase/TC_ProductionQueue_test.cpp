#include <gtest/gtest.h>
#include "model/ProductionQueue.h"

class ProductionQueueTest : public ::testing::Test {
protected:
    ProductionQueue queue;

    ProductionJob makeJob(int orderId, const std::string& sampleId = "S001",
                          int actualQty = 10, int totalTime = 300) {
        return ProductionJob{ orderId, sampleId, actualQty, totalTime };
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// empty / size
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ProductionQueueTest, Empty_InitialState_ReturnsTrue) {
    EXPECT_TRUE(queue.empty());
}

TEST_F(ProductionQueueTest, Empty_AfterEnqueue_ReturnsFalse) {
    queue.enqueue(makeJob(1));
    EXPECT_FALSE(queue.empty());
}

TEST_F(ProductionQueueTest, Size_InitialState_ReturnsZero) {
    EXPECT_EQ(queue.size(), 0u);
}

TEST_F(ProductionQueueTest, Size_AfterMultipleEnqueue_CorrectCount) {
    queue.enqueue(makeJob(1));
    queue.enqueue(makeJob(2));
    queue.enqueue(makeJob(3));
    EXPECT_EQ(queue.size(), 3u);
}

TEST_F(ProductionQueueTest, Size_AfterDequeue_Decrements) {
    queue.enqueue(makeJob(1));
    queue.enqueue(makeJob(2));
    queue.dequeue();
    EXPECT_EQ(queue.size(), 1u);
}

// ═══════════════════════════════════════════════════════════════════════════
// front
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ProductionQueueTest, Front_EmptyQueue_ReturnsNullopt) {
    EXPECT_FALSE(queue.front().has_value());
}

TEST_F(ProductionQueueTest, Front_SingleJob_ReturnsJobFields) {
    queue.enqueue(makeJob(1, "S001", 5, 100));
    auto job = queue.front();
    ASSERT_TRUE(job.has_value());
    EXPECT_EQ(job->orderId,   1);
    EXPECT_EQ(job->sampleId,  "S001");
    EXPECT_EQ(job->actualQty, 5);
    EXPECT_EQ(job->totalTime, 100);
}

TEST_F(ProductionQueueTest, Front_MultipleJobs_ReturnsFrontOnly) {
    queue.enqueue(makeJob(10));
    queue.enqueue(makeJob(20));
    EXPECT_EQ(queue.front()->orderId, 10);
}

TEST_F(ProductionQueueTest, Front_AfterDequeue_AdvancesToNext) {
    queue.enqueue(makeJob(10));
    queue.enqueue(makeJob(20));
    queue.dequeue();
    EXPECT_EQ(queue.front()->orderId, 20);
}

// ═══════════════════════════════════════════════════════════════════════════
// FIFO 순서 보장
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ProductionQueueTest, FIFO_ThreeJobs_DequeueInEnqueueOrder) {
    queue.enqueue(makeJob(10));
    queue.enqueue(makeJob(20));
    queue.enqueue(makeJob(30));

    EXPECT_EQ(queue.front()->orderId, 10); queue.dequeue();
    EXPECT_EQ(queue.front()->orderId, 20); queue.dequeue();
    EXPECT_EQ(queue.front()->orderId, 30); queue.dequeue();
    EXPECT_TRUE(queue.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// getWaiting — front 제외 대기 목록
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(ProductionQueueTest, GetWaiting_EmptyQueue_ReturnsEmpty) {
    EXPECT_TRUE(queue.getWaiting().empty());
}

TEST_F(ProductionQueueTest, GetWaiting_OneJob_ReturnsEmpty) {
    queue.enqueue(makeJob(1));
    EXPECT_TRUE(queue.getWaiting().empty());
}

TEST_F(ProductionQueueTest, GetWaiting_ThreeJobs_ReturnsTwoAfterFront) {
    queue.enqueue(makeJob(1));
    queue.enqueue(makeJob(2));
    queue.enqueue(makeJob(3));
    auto waiting = queue.getWaiting();
    ASSERT_EQ(waiting.size(), 2u);
    EXPECT_EQ(waiting[0].orderId, 2);
    EXPECT_EQ(waiting[1].orderId, 3);
}

TEST_F(ProductionQueueTest, GetWaiting_AfterDequeue_UpdatesCorrectly) {
    queue.enqueue(makeJob(1));
    queue.enqueue(makeJob(2));
    queue.enqueue(makeJob(3));
    queue.dequeue();
    auto waiting = queue.getWaiting();
    ASSERT_EQ(waiting.size(), 1u);
    EXPECT_EQ(waiting[0].orderId, 3);
}
