#include <gtest/gtest.h>
#include <filesystem>
#include <cstdio>
#include "data/SampleJsonRepository.h"

namespace fs = std::filesystem;

// ─── 테스트 픽스처 ────────────────────────────────────────────────────────────

class SampleJsonRepositoryTest : public ::testing::Test {
protected:
    const std::string kFile = "data/test_samples.json";

    SampleJsonRepository repo{ kFile };

    void SetUp() override {
        fs::remove(kFile);          // 이전 테스트 잔재 제거
        repo = SampleJsonRepository(kFile);
    }

    void TearDown() override {
        fs::remove(kFile);
    }

    Sample make(const std::string& id   = "S001",
                const std::string& name = "AlGaN",
                int   time  = 30,
                double yld  = 0.9,
                int   stock = 0) {
        return Sample{ id, name, time, yld, stock };
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// CREATE
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(SampleJsonRepositoryTest, Create_ValidSample_ReturnsTrue) {
    EXPECT_TRUE(repo.add(make()));
}

TEST_F(SampleJsonRepositoryTest, Create_ValidSample_ExistsAfterAdd) {
    repo.add(make("S001"));
    EXPECT_TRUE(repo.exists("S001"));
}

TEST_F(SampleJsonRepositoryTest, Create_DuplicateId_ReturnsFalse) {
    repo.add(make("S001"));
    EXPECT_FALSE(repo.add(make("S001", "GaN")));
}

TEST_F(SampleJsonRepositoryTest, Create_EmptyId_ReturnsFalse) {
    EXPECT_FALSE(repo.add(make("")));
}

TEST_F(SampleJsonRepositoryTest, Create_EmptyName_ReturnsFalse) {
    EXPECT_FALSE(repo.add(make("S001", "")));
}

TEST_F(SampleJsonRepositoryTest, Create_ZeroProductionTime_ReturnsFalse) {
    EXPECT_FALSE(repo.add(make("S001", "AlGaN", 0)));
}

TEST_F(SampleJsonRepositoryTest, Create_ZeroYield_ReturnsFalse) {
    EXPECT_FALSE(repo.add(make("S001", "AlGaN", 30, 0.0)));
}

TEST_F(SampleJsonRepositoryTest, Create_YieldExceedsOne_ReturnsFalse) {
    EXPECT_FALSE(repo.add(make("S001", "AlGaN", 30, 1.1)));
}

TEST_F(SampleJsonRepositoryTest, Create_YieldAtBoundary_ReturnsTrue) {
    EXPECT_TRUE(repo.add(make("S001", "AlGaN", 30, 1.0)));
}

TEST_F(SampleJsonRepositoryTest, Create_MultipleItems_AllExist) {
    repo.add(make("S001", "AlGaN"));
    repo.add(make("S002", "GaN"));
    repo.add(make("S003", "SiC"));
    EXPECT_EQ(repo.getAll().size(), 3u);
}

// ═══════════════════════════════════════════════════════════════════════════
// READ
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(SampleJsonRepositoryTest, Read_FindById_ExistingId_ReturnsSample) {
    repo.add(make("S001", "AlGaN", 30, 0.9, 50));
    auto result = repo.findById("S001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id,   "S001");
    EXPECT_EQ(result->name, "AlGaN");
    EXPECT_EQ(result->avgProductionTime, 30);
    EXPECT_DOUBLE_EQ(result->yield, 0.9);
    EXPECT_EQ(result->stock, 50);
}

TEST_F(SampleJsonRepositoryTest, Read_FindById_NonExistingId_ReturnsNullopt) {
    EXPECT_FALSE(repo.findById("NONE").has_value());
}

TEST_F(SampleJsonRepositoryTest, Read_FindByName_ExactMatch_ReturnsOne) {
    repo.add(make("S001", "AlGaN"));
    repo.add(make("S002", "GaN"));
    auto result = repo.findByName("AlGaN");
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].id, "S001");
}

TEST_F(SampleJsonRepositoryTest, Read_FindByName_PartialMatch_ReturnsMultiple) {
    repo.add(make("S001", "AlGaN"));
    repo.add(make("S002", "GaN"));
    repo.add(make("S003", "SiC"));
    // "GaN" 부분 일치: AlGaN, GaN
    auto result = repo.findByName("GaN");
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(SampleJsonRepositoryTest, Read_FindByName_NoMatch_ReturnsEmpty) {
    repo.add(make("S001", "AlGaN"));
    EXPECT_TRUE(repo.findByName("InP").empty());
}

TEST_F(SampleJsonRepositoryTest, Read_GetAll_EmptyRepo_ReturnsEmptyVector) {
    EXPECT_TRUE(repo.getAll().empty());
}

TEST_F(SampleJsonRepositoryTest, Read_GetAll_ReturnsAllItems) {
    repo.add(make("S001", "AlGaN"));
    repo.add(make("S002", "GaN"));
    EXPECT_EQ(repo.getAll().size(), 2u);
}

TEST_F(SampleJsonRepositoryTest, Read_Exists_ExistingId_ReturnsTrue) {
    repo.add(make("S001"));
    EXPECT_TRUE(repo.exists("S001"));
}

TEST_F(SampleJsonRepositoryTest, Read_Exists_NonExistingId_ReturnsFalse) {
    EXPECT_FALSE(repo.exists("S999"));
}

// ═══════════════════════════════════════════════════════════════════════════
// UPDATE
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(SampleJsonRepositoryTest, Update_UpdateStock_PositiveDelta_IncreasesStock) {
    repo.add(make("S001", "AlGaN", 30, 0.9, 10));
    repo.updateStock("S001", 20);
    EXPECT_EQ(repo.findById("S001")->stock, 30);
}

TEST_F(SampleJsonRepositoryTest, Update_UpdateStock_NegativeDelta_DecreasesStock) {
    repo.add(make("S001", "AlGaN", 30, 0.9, 50));
    repo.updateStock("S001", -30);
    EXPECT_EQ(repo.findById("S001")->stock, 20);
}

TEST_F(SampleJsonRepositoryTest, Update_UpdateStock_BelowZero_ClampsToZero) {
    repo.add(make("S001", "AlGaN", 30, 0.9, 5));
    repo.updateStock("S001", -100);
    EXPECT_EQ(repo.findById("S001")->stock, 0);
}

TEST_F(SampleJsonRepositoryTest, Update_UpdateStock_NonExistingId_ReturnsFalse) {
    EXPECT_FALSE(repo.updateStock("NONE", 10));
}

TEST_F(SampleJsonRepositoryTest, Update_UpdateSample_FieldsChanged) {
    repo.add(make("S001", "AlGaN", 30, 0.9, 0));
    Sample updated{ "S001", "AlGaN-v2", 45, 0.85, 100 };
    EXPECT_TRUE(repo.update(updated));
    auto result = repo.findById("S001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "AlGaN-v2");
    EXPECT_EQ(result->avgProductionTime, 45);
    EXPECT_DOUBLE_EQ(result->yield, 0.85);
    EXPECT_EQ(result->stock, 100);
}

TEST_F(SampleJsonRepositoryTest, Update_UpdateSample_NonExistingId_ReturnsFalse) {
    Sample s{ "NONE", "Ghost", 10, 0.5, 0 };
    EXPECT_FALSE(repo.update(s));
}

// ═══════════════════════════════════════════════════════════════════════════
// DELETE
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(SampleJsonRepositoryTest, Delete_Remove_ExistingId_ReturnsTrue) {
    repo.add(make("S001"));
    EXPECT_TRUE(repo.remove("S001"));
}

TEST_F(SampleJsonRepositoryTest, Delete_Remove_ExistingId_NoLongerExists) {
    repo.add(make("S001"));
    repo.remove("S001");
    EXPECT_FALSE(repo.exists("S001"));
}

TEST_F(SampleJsonRepositoryTest, Delete_Remove_DecreasesCount) {
    repo.add(make("S001"));
    repo.add(make("S002"));
    repo.remove("S001");
    EXPECT_EQ(repo.getAll().size(), 1u);
}

TEST_F(SampleJsonRepositoryTest, Delete_Remove_NonExistingId_ReturnsFalse) {
    EXPECT_FALSE(repo.remove("NONE"));
}

TEST_F(SampleJsonRepositoryTest, Delete_Remove_RemainingItemsIntact) {
    repo.add(make("S001", "AlGaN"));
    repo.add(make("S002", "GaN"));
    repo.remove("S001");
    auto all = repo.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id, "S002");
}

// ═══════════════════════════════════════════════════════════════════════════
// PERSISTENCE (저장 / 재로드)
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(SampleJsonRepositoryTest, Persist_AddAndReload_DataIntact) {
    repo.add(make("S001", "AlGaN", 30, 0.9, 50));
    repo.add(make("S002", "GaN",   20, 0.8, 10));

    // 새 인스턴스로 파일을 다시 읽음
    SampleJsonRepository reloaded(kFile);
    ASSERT_EQ(reloaded.getAll().size(), 2u);
    auto s = reloaded.findById("S001");
    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(s->name,  "AlGaN");
    EXPECT_EQ(s->stock, 50);
}

TEST_F(SampleJsonRepositoryTest, Persist_UpdateStockAndReload_StockPersisted) {
    repo.add(make("S001", "AlGaN", 30, 0.9, 0));
    repo.updateStock("S001", 100);

    SampleJsonRepository reloaded(kFile);
    EXPECT_EQ(reloaded.findById("S001")->stock, 100);
}

TEST_F(SampleJsonRepositoryTest, Persist_RemoveAndReload_ItemAbsent) {
    repo.add(make("S001"));
    repo.add(make("S002"));
    repo.remove("S001");

    SampleJsonRepository reloaded(kFile);
    EXPECT_FALSE(reloaded.exists("S001"));
    EXPECT_TRUE(reloaded.exists("S002"));
}
