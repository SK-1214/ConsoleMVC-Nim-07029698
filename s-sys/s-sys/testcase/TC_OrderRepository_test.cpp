#include <gtest/gtest.h>
#include "model/OrderRepository.h"

class OrderRepositoryTest : public ::testing::Test {
protected:
    OrderRepository repo;

    Order makeOrder(const std::string& sampleId   = "S001",
                    const std::string& customer   = "고객A",
                    int                qty        = 10,
                    OrderStatus        status     = OrderStatus::RESERVED) {
        Order o;
        o.sampleId     = sampleId;
        o.customerName = customer;
        o.quantity     = qty;
        o.status       = status;
        return o;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// add
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderRepositoryTest, Add_ValidOrder_ReturnsPositiveId) {
    int id = repo.add(makeOrder());
    EXPECT_GT(id, 0);
}

TEST_F(OrderRepositoryTest, Add_TwoOrders_IdsAreUnique) {
    int id1 = repo.add(makeOrder("S001", "고객A"));
    int id2 = repo.add(makeOrder("S002", "고객B"));
    EXPECT_NE(id1, id2);
}

TEST_F(OrderRepositoryTest, Add_MultipleOrders_CountIncrements) {
    repo.add(makeOrder("S001"));
    repo.add(makeOrder("S002"));
    EXPECT_EQ(repo.getAll().size(), 2u);
}

TEST_F(OrderRepositoryTest, Add_OrderFields_StoredCorrectly) {
    int id = repo.add(makeOrder("S001", "홍길동", 25));
    auto result = repo.findById(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->sampleId,     "S001");
    EXPECT_EQ(result->customerName, "홍길동");
    EXPECT_EQ(result->quantity,     25);
    EXPECT_EQ(result->status,       OrderStatus::RESERVED);
}

// ═══════════════════════════════════════════════════════════════════════════
// findById
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderRepositoryTest, FindById_ExistingId_ReturnsOrder) {
    int id = repo.add(makeOrder());
    auto result = repo.findById(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, id);
}

TEST_F(OrderRepositoryTest, FindById_NonExistingId_ReturnsNullopt) {
    EXPECT_FALSE(repo.findById(9999).has_value());
}

// ═══════════════════════════════════════════════════════════════════════════
// getAll
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderRepositoryTest, GetAll_EmptyRepo_ReturnsEmptyVector) {
    EXPECT_TRUE(repo.getAll().empty());
}

TEST_F(OrderRepositoryTest, GetAll_AfterAdd_ReturnsAllOrders) {
    repo.add(makeOrder("S001"));
    repo.add(makeOrder("S002"));
    repo.add(makeOrder("S003"));
    EXPECT_EQ(repo.getAll().size(), 3u);
}

// ═══════════════════════════════════════════════════════════════════════════
// getByStatus
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderRepositoryTest, GetByStatus_Reserved_ReturnsOnlyReserved) {
    int id1 = repo.add(makeOrder("S001"));
    int id2 = repo.add(makeOrder("S002"));
    repo.updateStatus(id2, OrderStatus::CONFIRMED);

    auto reserved = repo.getByStatus(OrderStatus::RESERVED);
    ASSERT_EQ(reserved.size(), 1u);
    EXPECT_EQ(reserved[0].id, id1);
}

TEST_F(OrderRepositoryTest, GetByStatus_NoMatch_ReturnsEmpty) {
    repo.add(makeOrder());
    EXPECT_TRUE(repo.getByStatus(OrderStatus::RELEASE).empty());
}

TEST_F(OrderRepositoryTest, GetByStatus_MultipleStatuses_EachCorrectCount) {
    int id1 = repo.add(makeOrder("S001"));
    int id2 = repo.add(makeOrder("S002"));
    int id3 = repo.add(makeOrder("S003"));
    repo.updateStatus(id2, OrderStatus::PRODUCING);
    repo.updateStatus(id3, OrderStatus::CONFIRMED);

    EXPECT_EQ(repo.getByStatus(OrderStatus::RESERVED).size(),  1u);
    EXPECT_EQ(repo.getByStatus(OrderStatus::PRODUCING).size(), 1u);
    EXPECT_EQ(repo.getByStatus(OrderStatus::CONFIRMED).size(), 1u);
}

// ═══════════════════════════════════════════════════════════════════════════
// updateStatus
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(OrderRepositoryTest, UpdateStatus_ExistingId_ReturnsTrue) {
    int id = repo.add(makeOrder());
    EXPECT_TRUE(repo.updateStatus(id, OrderStatus::CONFIRMED));
}

TEST_F(OrderRepositoryTest, UpdateStatus_ExistingId_StatusChanged) {
    int id = repo.add(makeOrder());
    repo.updateStatus(id, OrderStatus::CONFIRMED);
    EXPECT_EQ(repo.findById(id)->status, OrderStatus::CONFIRMED);
}

TEST_F(OrderRepositoryTest, UpdateStatus_NonExistingId_ReturnsFalse) {
    EXPECT_FALSE(repo.updateStatus(9999, OrderStatus::CONFIRMED));
}

TEST_F(OrderRepositoryTest, UpdateStatus_FullTransitionChain_AllSucceed) {
    int id = repo.add(makeOrder());
    EXPECT_TRUE(repo.updateStatus(id, OrderStatus::PRODUCING));
    EXPECT_TRUE(repo.updateStatus(id, OrderStatus::CONFIRMED));
    EXPECT_TRUE(repo.updateStatus(id, OrderStatus::RELEASE));
    EXPECT_EQ(repo.findById(id)->status, OrderStatus::RELEASE);
}

TEST_F(OrderRepositoryTest, UpdateStatus_ReservedToRejected_StatusChanged) {
    int id = repo.add(makeOrder());
    repo.updateStatus(id, OrderStatus::REJECTED);
    EXPECT_EQ(repo.findById(id)->status, OrderStatus::REJECTED);
}
