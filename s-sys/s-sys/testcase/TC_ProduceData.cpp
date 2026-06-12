#include <gtest/gtest.h>
#include <filesystem>
#include <thread>
#include <chrono>
#include "data/OrderJsonRepository.h"
#include "data/ProductionQueueJsonRepository.h"
#include "model/ProductionQueue.h"
#include "controller/ProductionController.h"
#include "model/OrderRepository.h"
#include "model/SampleRepository.h"

namespace fs = std::filesystem;

static const std::string TC_PRODUCE_DIR        = "test_producedata_tmp";
static const std::string TC_PRODUCE_SAMPLE_DIR = "test_producedata_sample_tmp";

// ─── 공통 헬퍼 ────────────────────────────────────────────────────────────────

static Order makeOrder(const std::string& sampleId   = "S001",
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

static ProductionJob makeJob(int orderId, const std::string& sampleId = "S001",
                             int actualQty = 10, int totalTime = 300) {
    return ProductionJob{ orderId, sampleId, actualQty, totalTime };
}

// ═══════════════════════════════════════════════════════════════════════════
// OrderJsonRepository — JSON 영속성
// ═══════════════════════════════════════════════════════════════════════════

class OrderJsonRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories(TC_PRODUCE_DIR);
    }
    void TearDown() override {
        fs::remove_all(TC_PRODUCE_DIR);
    }
};

TEST_F(OrderJsonRepositoryTest, Add_SingleOrder_PersistedToFile) {
    {
        OrderJsonRepository repo(TC_PRODUCE_DIR);
        repo.add(makeOrder("S001", "홍길동", 10));
    }
    // 새 인스턴스로 파일 재로드
    OrderJsonRepository reloaded(TC_PRODUCE_DIR);
    ASSERT_EQ(reloaded.getAll().size(), 1u);
    EXPECT_EQ(reloaded.getAll()[0].sampleId,     "S001");
    EXPECT_EQ(reloaded.getAll()[0].customerName, "홍길동");
    EXPECT_EQ(reloaded.getAll()[0].quantity,     10);
}

TEST_F(OrderJsonRepositoryTest, Add_MultipleOrders_AllPersistedWithUniqueIds) {
    {
        OrderJsonRepository repo(TC_PRODUCE_DIR);
        repo.add(makeOrder("S001"));
        repo.add(makeOrder("S002"));
        repo.add(makeOrder("S003"));
    }
    OrderJsonRepository reloaded(TC_PRODUCE_DIR);
    auto all = reloaded.getAll();
    ASSERT_EQ(all.size(), 3u);
    EXPECT_NE(all[0].id, all[1].id);
    EXPECT_NE(all[1].id, all[2].id);
}

TEST_F(OrderJsonRepositoryTest, Reload_NextIdContinuesFromLastId) {
    int lastId;
    {
        OrderJsonRepository repo(TC_PRODUCE_DIR);
        repo.add(makeOrder("S001"));
        lastId = repo.add(makeOrder("S002"));
    }
    OrderJsonRepository reloaded(TC_PRODUCE_DIR);
    int newId = reloaded.add(makeOrder("S003"));
    EXPECT_GT(newId, lastId);
}

TEST_F(OrderJsonRepositoryTest, FindById_AfterReload_ReturnsCorrectOrder) {
    int savedId;
    {
        OrderJsonRepository repo(TC_PRODUCE_DIR);
        savedId = repo.add(makeOrder("S001", "테스터", 25));
    }
    OrderJsonRepository reloaded(TC_PRODUCE_DIR);
    auto result = reloaded.findById(savedId);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->sampleId,     "S001");
    EXPECT_EQ(result->customerName, "테스터");
    EXPECT_EQ(result->quantity,     25);
}

TEST_F(OrderJsonRepositoryTest, GetByStatus_AfterReload_FiltersCorrectly) {
    {
        OrderJsonRepository repo(TC_PRODUCE_DIR);
        int id1 = repo.add(makeOrder("S001"));
        int id2 = repo.add(makeOrder("S002"));
        repo.updateStatus(id2, OrderStatus::CONFIRMED);
    }
    OrderJsonRepository reloaded(TC_PRODUCE_DIR);
    EXPECT_EQ(reloaded.getByStatus(OrderStatus::RESERVED).size(),  1u);
    EXPECT_EQ(reloaded.getByStatus(OrderStatus::CONFIRMED).size(), 1u);
}

TEST_F(OrderJsonRepositoryTest, UpdateStatus_PersistedAcrossReload) {
    int id;
    {
        OrderJsonRepository repo(TC_PRODUCE_DIR);
        id = repo.add(makeOrder("S001"));
        repo.updateStatus(id, OrderStatus::PRODUCING);
    }
    OrderJsonRepository reloaded(TC_PRODUCE_DIR);
    auto result = reloaded.findById(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, OrderStatus::PRODUCING);
}

TEST_F(OrderJsonRepositoryTest, UpdateStatus_AllTransitions_EachPersisted) {
    OrderJsonRepository repo(TC_PRODUCE_DIR);
    int id = repo.add(makeOrder());

    for (auto status : { OrderStatus::PRODUCING, OrderStatus::CONFIRMED, OrderStatus::RELEASE }) {
        repo.updateStatus(id, status);
        OrderJsonRepository reloaded(TC_PRODUCE_DIR);
        EXPECT_EQ(reloaded.findById(id)->status, status);
    }
}

TEST_F(OrderJsonRepositoryTest, UpdateStatus_NonExistingId_ReturnsFalse) {
    OrderJsonRepository repo(TC_PRODUCE_DIR);
    EXPECT_FALSE(repo.updateStatus(9999, OrderStatus::CONFIRMED));
}

TEST_F(OrderJsonRepositoryTest, Reload_EmptyDir_ReturnsEmptyList) {
    OrderJsonRepository repo(TC_PRODUCE_DIR);
    EXPECT_TRUE(repo.getAll().empty());
    EXPECT_EQ(repo.getNextId(), 1);
}

// ═══════════════════════════════════════════════════════════════════════════
// ProductionQueueJsonRepository — JSON 영속성
// ═══════════════════════════════════════════════════════════════════════════

class ProductionQueueJsonRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories(TC_PRODUCE_DIR);
    }
    void TearDown() override {
        fs::remove_all(TC_PRODUCE_DIR);
    }
};

TEST_F(ProductionQueueJsonRepositoryTest, SaveAndLoad_SingleJob_RoundtripCorrect) {
    ProductionQueueJsonRepository repo(TC_PRODUCE_DIR);
    std::vector<ProductionJob> jobs = { makeJob(1, "S001", 10, 300) };
    EXPECT_TRUE(repo.save(jobs));

    auto loaded = repo.loadJobs();
    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].orderId,   1);
    EXPECT_EQ(loaded[0].sampleId,  "S001");
    EXPECT_EQ(loaded[0].actualQty, 10);
    EXPECT_EQ(loaded[0].totalTime, 300);
}

TEST_F(ProductionQueueJsonRepositoryTest, SaveAndLoad_MultipleJobs_OrderPreserved) {
    ProductionQueueJsonRepository repo(TC_PRODUCE_DIR);
    std::vector<ProductionJob> jobs = {
        makeJob(10, "S001", 5,  100),
        makeJob(20, "S002", 8,  200),
        makeJob(30, "S003", 12, 400)
    };
    repo.save(jobs);

    auto loaded = repo.loadJobs();
    ASSERT_EQ(loaded.size(), 3u);
    EXPECT_EQ(loaded[0].orderId, 10);
    EXPECT_EQ(loaded[1].orderId, 20);
    EXPECT_EQ(loaded[2].orderId, 30);
}

TEST_F(ProductionQueueJsonRepositoryTest, SaveEmptyList_LoadReturnsEmpty) {
    ProductionQueueJsonRepository repo(TC_PRODUCE_DIR);
    repo.save({});
    EXPECT_TRUE(repo.loadJobs().empty());
}

TEST_F(ProductionQueueJsonRepositoryTest, Load_NoFileExists_ReturnsEmpty) {
    ProductionQueueJsonRepository repo(TC_PRODUCE_DIR);
    EXPECT_TRUE(repo.loadJobs().empty());
}

TEST_F(ProductionQueueJsonRepositoryTest, Overwrite_SaveAgain_PreviousDataReplaced) {
    ProductionQueueJsonRepository repo(TC_PRODUCE_DIR);
    repo.save({ makeJob(1), makeJob(2) });
    repo.save({ makeJob(99, "S005", 3, 50) });

    auto loaded = repo.loadJobs();
    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].orderId, 99);
}

// ═══════════════════════════════════════════════════════════════════════════
// ProductionQueue — JSON 백업 기반 영속성
// ═══════════════════════════════════════════════════════════════════════════

class ProductionQueuePersistTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories(TC_PRODUCE_DIR);
    }
    void TearDown() override {
        fs::remove_all(TC_PRODUCE_DIR);
    }
};

TEST_F(ProductionQueuePersistTest, Enqueue_PersistedAcrossInstances) {
    {
        ProductionQueue q(TC_PRODUCE_DIR);
        q.enqueue(makeJob(1, "S001", 10, 300));
    }
    ProductionQueue reloaded(TC_PRODUCE_DIR);
    ASSERT_FALSE(reloaded.empty());
    EXPECT_EQ(reloaded.front()->orderId, 1);
}

TEST_F(ProductionQueuePersistTest, Enqueue_MultipleJobs_FIFOOrderAfterReload) {
    {
        ProductionQueue q(TC_PRODUCE_DIR);
        q.enqueue(makeJob(10));
        q.enqueue(makeJob(20));
        q.enqueue(makeJob(30));
    }
    ProductionQueue reloaded(TC_PRODUCE_DIR);
    EXPECT_EQ(reloaded.size(), 3u);
    EXPECT_EQ(reloaded.front()->orderId, 10);
}

TEST_F(ProductionQueuePersistTest, Dequeue_PersistedAcrossInstances) {
    {
        ProductionQueue q(TC_PRODUCE_DIR);
        q.enqueue(makeJob(10));
        q.enqueue(makeJob(20));
        q.dequeue();
    }
    ProductionQueue reloaded(TC_PRODUCE_DIR);
    ASSERT_EQ(reloaded.size(), 1u);
    EXPECT_EQ(reloaded.front()->orderId, 20);
}

TEST_F(ProductionQueuePersistTest, DequeueAll_EmptyQueuePersistedAcrossInstances) {
    {
        ProductionQueue q(TC_PRODUCE_DIR);
        q.enqueue(makeJob(1));
        q.dequeue();
    }
    ProductionQueue reloaded(TC_PRODUCE_DIR);
    EXPECT_TRUE(reloaded.empty());
}

TEST_F(ProductionQueuePersistTest, Reload_EmptyDir_StartsEmpty) {
    ProductionQueue q(TC_PRODUCE_DIR);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

// ═══════════════════════════════════════════════════════════════════════════
// ProductionController — getCurrentProductionProgress / tickProduction
// ═══════════════════════════════════════════════════════════════════════════

class ProductionControllerProgressTest : public ::testing::Test {
protected:
    ProductionQueue  queue{ TC_PRODUCE_DIR };
    OrderRepository  orderRepo{ TC_PRODUCE_DIR };
    SampleRepository sampleRepo{ TC_PRODUCE_SAMPLE_DIR };
    ProductionController ctrl{ queue, orderRepo, sampleRepo };

    void SetUp() override {
        fs::create_directories(TC_PRODUCE_DIR);
        fs::create_directories(TC_PRODUCE_SAMPLE_DIR);
    }
    void TearDown() override {
        fs::remove_all(TC_PRODUCE_DIR);
        fs::remove_all(TC_PRODUCE_SAMPLE_DIR);
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
};

// ─── getCurrentProductionProgress ────────────────────────────────────────────

TEST_F(ProductionControllerProgressTest, GetCurrentProductionProgress_EmptyQueue_ReturnsNullopt) {
    EXPECT_FALSE(ctrl.getCurrentProductionProgress().has_value());
}

TEST_F(ProductionControllerProgressTest, GetCurrentProductionProgress_JobInQueue_ReturnsProgress) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 720));

    auto progress = ctrl.getCurrentProductionProgress();
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->job.orderId,   id);
    EXPECT_EQ(progress->job.actualQty, 12);
    EXPECT_EQ(progress->job.totalTime, 720);
}

TEST_F(ProductionControllerProgressTest, GetCurrentProductionProgress_ElapsedSecNonNegative) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 720));

    auto progress = ctrl.getCurrentProductionProgress();
    ASSERT_TRUE(progress.has_value());
    EXPECT_GE(progress->elapsedSec, 0);
}

TEST_F(ProductionControllerProgressTest, GetCurrentProductionProgress_RemainingSecNotExceedsTotalTime) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 720));

    auto progress = ctrl.getCurrentProductionProgress();
    ASSERT_TRUE(progress.has_value());
    EXPECT_LE(progress->remainingSec, progress->job.totalTime);
}

TEST_F(ProductionControllerProgressTest, GetCurrentProductionProgress_ElapsedPlusRemainingEqualsTotalTime) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 720));

    auto p1 = ctrl.getCurrentProductionProgress();
    auto p2 = ctrl.getCurrentProductionProgress();
    ASSERT_TRUE(p1.has_value() && p2.has_value());
    // elapsed + remaining = totalTime (잔여시간은 최소 0으로 클램핑됨)
    EXPECT_EQ(p1->elapsedSec + p1->remainingSec, p1->job.totalTime);
}

TEST_F(ProductionControllerProgressTest, GetCurrentProductionProgress_AfterComplete_NextJobStartsFresh) {
    addSample("S001");
    int id1 = addProducingOrder("S001", 10);
    int id2 = addProducingOrder("S001",  5);
    queue.enqueue(makeJob(id1, "S001", 12, 720));
    queue.enqueue(makeJob(id2, "S001",  6, 360));

    // 첫 번째 작업 완료 후 두 번째 작업의 경과시간은 0에 가까워야 함
    ctrl.completeCurrentProduction();
    auto progress = ctrl.getCurrentProductionProgress();
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->job.orderId, id2);
    EXPECT_GE(progress->elapsedSec, 0);
    EXPECT_LE(progress->remainingSec, 360);
}

// ─── tickProduction ───────────────────────────────────────────────────────────

TEST_F(ProductionControllerProgressTest, TickProduction_EmptyQueue_ReturnsNullopt) {
    EXPECT_FALSE(ctrl.tickProduction().has_value());
}

TEST_F(ProductionControllerProgressTest, TickProduction_TimeNotElapsed_ReturnsNullopt) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 99999));  // 매우 긴 totalTime

    EXPECT_FALSE(ctrl.tickProduction().has_value());
    EXPECT_FALSE(queue.empty());  // 아직 완료 안 됨
}

TEST_F(ProductionControllerProgressTest, TickProduction_TotalTimeZero_CompletesImmediately) {
    // totalTime=0 이면 경과시간(>=0) >= totalTime(0) → 즉시 완료
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 0));

    auto completed = ctrl.tickProduction();
    ASSERT_TRUE(completed.has_value());
    EXPECT_EQ(completed->orderId, id);
}

TEST_F(ProductionControllerProgressTest, TickProduction_Completes_OrderStatusConfirmed) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 0));

    ctrl.tickProduction();

    EXPECT_EQ(orderRepo.findById(id)->status, OrderStatus::CONFIRMED);
}

TEST_F(ProductionControllerProgressTest, TickProduction_Completes_QueueAdvances) {
    addSample("S001");
    int id1 = addProducingOrder("S001", 10);
    int id2 = addProducingOrder("S001",  5);
    queue.enqueue(makeJob(id1, "S001", 12, 0));
    queue.enqueue(makeJob(id2, "S001",  6, 99999));

    ctrl.tickProduction();

    EXPECT_EQ(queue.size(), 1u);
    EXPECT_EQ(ctrl.getCurrentProduction()->orderId, id2);
}

TEST_F(ProductionControllerProgressTest, TickProduction_Completes_StockIncreasedByActualQty) {
    addSample("S001", 5);
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 0));

    ctrl.tickProduction();

    EXPECT_EQ(sampleRepo.getStock("S001"), 5 + 12);
}

TEST_F(ProductionControllerProgressTest, TickProduction_AfterSleep_LongJobElapsed_Completes) {
    addSample("S001");
    int id = addProducingOrder("S001", 10);
    queue.enqueue(makeJob(id, "S001", 12, 1));  // totalTime = 1초

    // 시작 시각 등록을 위해 tick 한 번 호출
    ctrl.tickProduction();

    // 1초 대기 후 완료 확인
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto completed = ctrl.tickProduction();
    // 완료됐거나 이미 첫 tick에서 완료됐을 수 있으므로 queue가 비어있음을 검증
    EXPECT_TRUE(queue.empty());
    (void)completed;
}
