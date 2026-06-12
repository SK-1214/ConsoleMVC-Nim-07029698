#include <gtest/gtest.h>
#include <filesystem>
#include "data/SampleJsonRepository.h"

namespace fs = std::filesystem;

// sampledata 경로 (main.cpp 위치 기준 상대경로)
static const std::string SAMPLE_DIR = "sampledata";

// 사전 생성된 시료 ID (DummyDataGenerator로 미리 생성)
static const std::vector<std::string> PRE_IDS = {
    "S001", "S002", "S003", "S004", "S005"
};

// 테스트 중 신규 생성할 시료 ID
static const std::vector<std::string> NEW_IDS = {
    "T001", "T002", "T003", "T004", "T005"
};

// ─── 픽스처 ──────────────────────────────────────────────────────────────────

class TC_SampleTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // 이전 실행 잔여 T001-T005 정리
        SampleJsonRepository repo(SAMPLE_DIR);
        for (const auto& id : NEW_IDS)
            repo.remove(id);
    }

    static void TearDownTestSuite() {
        // 테스트 후 T001-T005 최종 정리
        SampleJsonRepository repo(SAMPLE_DIR);
        for (const auto& id : NEW_IDS)
            repo.remove(id);
    }

    // 매 테스트마다 파일 기반 최신 상태로 로드
    SampleJsonRepository repo{ SAMPLE_DIR };
};

// ═══════════════════════════════════════════════════════════════════════════
// READ — 사전 생성된 시료 5개 조회
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Read_PreExisting_AllLoaded) {
    auto all = repo.getAll();
    EXPECT_GE(all.size(), PRE_IDS.size())
        << "sampledata/ 에 S001-S005 파일이 있어야 합니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Read_PreExisting_EachIdFound) {
    for (const auto& id : PRE_IDS) {
        auto s = repo.findById(id);
        ASSERT_TRUE(s.has_value()) << "ID 조회 실패: " << id;
        EXPECT_EQ(s->id, id);
        EXPECT_FALSE(s->name.empty());
        EXPECT_GT(s->avgProductionTime, 0);
        EXPECT_GT(s->yield, 0.0);
        EXPECT_LE(s->yield, 1.0);
    }
}

TEST_F(TC_SampleTest, TC_Sample_Read_PreExisting_FindByName) {
    // "웨이퍼" 키워드로 S001-S005 전부 검색 가능해야 함
    auto result = repo.findByName("웨이퍼");
    EXPECT_GE(result.size(), PRE_IDS.size());
}

// ═══════════════════════════════════════════════════════════════════════════
// UPDATE — 사전 생성된 시료 5개 수정
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Update_PreExisting_StockDelta) {
    for (const auto& id : PRE_IDS) {
        auto before = repo.findById(id);
        ASSERT_TRUE(before.has_value());
        int prevStock = before->stock;

        EXPECT_TRUE(repo.updateStock(id, 10));

        SampleJsonRepository reloaded(SAMPLE_DIR);
        auto after = reloaded.findById(id);
        ASSERT_TRUE(after.has_value());
        EXPECT_EQ(after->stock, prevStock + 10)
            << id << " 재고 갱신 불일치";
    }
}

TEST_F(TC_SampleTest, TC_Sample_Update_PreExisting_ProductionTimeChanged) {
    for (const auto& id : PRE_IDS) {
        auto orig = repo.findById(id);
        ASSERT_TRUE(orig.has_value());

        Sample updated = *orig;
        updated.avgProductionTime = orig->avgProductionTime + 100;
        EXPECT_TRUE(repo.update(updated));

        SampleJsonRepository reloaded(SAMPLE_DIR);
        auto result = reloaded.findById(id);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->avgProductionTime, orig->avgProductionTime + 100)
            << id << " 생산시간 갱신 불일치";
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// CREATE — 신규 시료 T001-T005 생성
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Create_NewSamples_AddSucceeds) {
    int before = static_cast<int>(repo.getAll().size());

    for (int i = 0; i < 5; ++i) {
        Sample s;
        s.id                = NEW_IDS[i];
        s.name              = "테스트 시료 " + std::to_string(i + 1);
        s.avgProductionTime = 600 + i * 60;
        s.yield             = 0.80 + i * 0.02;
        s.stock             = 50  + i * 10;
        EXPECT_TRUE(repo.add(s)) << "추가 실패: " << s.id;
    }

    EXPECT_EQ(static_cast<int>(repo.getAll().size()), before + 5);
}

TEST_F(TC_SampleTest, TC_Sample_Create_NewSamples_DuplicateRejected) {
    Sample dup{ "T001", "중복시료", 300, 0.9, 0 };
    EXPECT_FALSE(repo.add(dup)) << "중복 ID 추가가 거부되어야 합니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Create_NewSamples_InvalidRejected) {
    EXPECT_FALSE(repo.add({ "",     "이름없음", 300, 0.9, 0 })) << "빈 ID 거부";
    EXPECT_FALSE(repo.add({ "T999", "",         300, 0.9, 0 })) << "빈 이름 거부";
    EXPECT_FALSE(repo.add({ "T999", "시료",       0, 0.9, 0 })) << "생산시간 0 거부";
    EXPECT_FALSE(repo.add({ "T999", "시료",     300, 0.0, 0 })) << "수율 0 거부";
    EXPECT_FALSE(repo.add({ "T999", "시료",     300, 1.1, 0 })) << "수율 초과 거부";
}

// ═══════════════════════════════════════════════════════════════════════════
// READ — 신규 생성된 T001-T005 조회
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Read_NewSamples_FieldsMatch) {
    for (int i = 0; i < 5; ++i) {
        auto s = repo.findById(NEW_IDS[i]);
        ASSERT_TRUE(s.has_value()) << "조회 실패: " << NEW_IDS[i];
        EXPECT_EQ(s->id,   NEW_IDS[i]);
        EXPECT_EQ(s->name, "테스트 시료 " + std::to_string(i + 1));
        EXPECT_EQ(s->avgProductionTime, 600 + i * 60);
        EXPECT_DOUBLE_EQ(s->yield, 0.80 + i * 0.02);
        EXPECT_EQ(s->stock, 50 + i * 10);
    }
}

TEST_F(TC_SampleTest, TC_Sample_Read_NewSamples_AllExist) {
    for (const auto& id : NEW_IDS)
        EXPECT_TRUE(repo.exists(id)) << id << " 존재해야 합니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Read_NewSamples_FindByName) {
    auto result = repo.findByName("테스트 시료");
    EXPECT_EQ(result.size(), NEW_IDS.size());
}

// ═══════════════════════════════════════════════════════════════════════════
// UPDATE — 신규 생성된 T001-T005 수정
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Update_NewSamples_StockIncreased) {
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(repo.updateStock(NEW_IDS[i], 100));

        SampleJsonRepository reloaded(SAMPLE_DIR);
        auto s = reloaded.findById(NEW_IDS[i]);
        ASSERT_TRUE(s.has_value());
        EXPECT_EQ(s->stock, 50 + i * 10 + 100)
            << NEW_IDS[i] << " 재고 갱신 불일치";
    }
}

TEST_F(TC_SampleTest, TC_Sample_Update_NewSamples_FieldsPersisted) {
    for (const auto& id : NEW_IDS) {
        auto orig = repo.findById(id);
        ASSERT_TRUE(orig.has_value());

        Sample updated = *orig;
        updated.avgProductionTime = 9999;
        updated.yield             = 0.95;
        EXPECT_TRUE(repo.update(updated));

        SampleJsonRepository reloaded(SAMPLE_DIR);
        auto result = reloaded.findById(id);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->avgProductionTime, 9999);
        EXPECT_DOUBLE_EQ(result->yield, 0.95);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// DELETE — 신규 생성된 T001-T005 삭제
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Delete_NewSamples_RemoveSucceeds) {
    for (const auto& id : NEW_IDS)
        EXPECT_TRUE(repo.remove(id)) << "삭제 실패: " << id;
}

TEST_F(TC_SampleTest, TC_Sample_Delete_NewSamples_NoLongerExists) {
    for (const auto& id : NEW_IDS)
        EXPECT_FALSE(repo.exists(id)) << id << " 는 삭제 후 존재하면 안됩니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Delete_NewSamples_CountDecreased) {
    SampleJsonRepository reloaded(SAMPLE_DIR);
    for (const auto& id : NEW_IDS)
        EXPECT_FALSE(reloaded.exists(id));
    // 사전 시료(S001-S005)는 유지되어야 함
    for (const auto& id : PRE_IDS)
        EXPECT_TRUE(reloaded.exists(id)) << id << " 사전 시료가 삭제되었습니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Delete_NonExisting_ReturnsFalse) {
    EXPECT_FALSE(repo.remove("NONE"));
}
