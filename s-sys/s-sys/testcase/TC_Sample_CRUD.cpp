#include <gtest/gtest.h>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "data/SampleJsonRepository.h"

namespace fs = std::filesystem;

// ─── 상수 ─────────────────────────────────────────────────────────────────────

static const std::string SAMPLE_DIR = "sampledata";

static const std::vector<std::string> PRE_IDS = {
    "S001", "S002", "S003", "S004", "S005"
};

static const std::vector<std::string> NEW_IDS = {
    "T001", "T002", "T003", "T004", "T005"
};

// DummyDataGenerator 와 동일한 반도체 소재 이름 풀 (S001-S005 기본 생성용)
static const std::vector<std::string> MATERIAL_NAMES = {
    "실리콘 웨이퍼 8인치",
    "실리콘 웨이퍼 12인치",
    "GAA 공정 웨이퍼 12인치",
    "SiC 파워 웨이퍼 8인치",
    "GaN-on-Si 파워 웨이퍼 8인치",
};

// ─── 출력 헬퍼 ────────────────────────────────────────────────────────────────

static void printSample(const Sample& s) {
    std::cout << "  " << std::left << std::setw(5) << s.id
              << " | " << std::setw(30) << s.name
              << " | 생산시간: " << std::setw(6) << s.avgProductionTime << "초"
              << " | 수율: " << std::fixed << std::setprecision(2) << s.yield
              << " | 재고: " << s.stock
              << "\n";
}

static void printDiff(const Sample& before, const Sample& after) {
    std::cout << "  [UPDATE] " << before.id;
    bool changed = false;

    if (before.stock != after.stock) {
        std::cout << " | 재고: " << before.stock << " → " << after.stock;
        changed = true;
    }
    if (before.avgProductionTime != after.avgProductionTime) {
        std::cout << " | 생산시간: " << before.avgProductionTime
                  << " → " << after.avgProductionTime << "초";
        changed = true;
    }
    if (std::abs(before.yield - after.yield) > 1e-9) {
        std::cout << std::fixed << std::setprecision(2)
                  << " | 수율: " << before.yield << " → " << after.yield;
        changed = true;
    }
    if (before.name != after.name) {
        std::cout << " | 이름: " << before.name << " → " << after.name;
        changed = true;
    }
    if (!changed) std::cout << " (변경 없음)";
    std::cout << "\n";
}

// ─── 픽스처 ──────────────────────────────────────────────────────────────────

class TC_SampleTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // 이전 실행 잔여 T001-T005 정리
        {
            SampleJsonRepository repo(SAMPLE_DIR);
            for (const auto& id : NEW_IDS)
                repo.remove(id);
        }

        // sampledata/ 에 S001-S005 가 없으면 DummyDataGenerator 로직으로 자동 생성
        SampleJsonRepository repo(SAMPLE_DIR);
        bool hasAll = true;
        for (const auto& id : PRE_IDS)
            if (!repo.exists(id)) { hasAll = false; break; }

        if (!hasAll)
            generateDefaultSamples();
    }

    static void TearDownTestSuite() {
        // 테스트 후 T001-T005 최종 정리
        SampleJsonRepository repo(SAMPLE_DIR);
        for (const auto& id : NEW_IDS)
            repo.remove(id);
    }

    // 매 테스트마다 파일 기반 최신 상태로 로드
    SampleJsonRepository repo{ SAMPLE_DIR };

private:
    // DummyDataGenerator 와 동일한 소재 목록·범위로 S001-S005 생성
    static void generateDefaultSamples() {
        struct DefaultData {
            std::string id, name;
            int    avgProductionTime;
            double yield;
            int    stock;
        };

        static const std::vector<DefaultData> defaults = {
            {"S001", MATERIAL_NAMES[0], 3600, 0.92, 150},
            {"S002", MATERIAL_NAMES[1], 5400, 0.88, 200},
            {"S003", MATERIAL_NAMES[2], 7200, 0.85,  80},
            {"S004", MATERIAL_NAMES[3], 4800, 0.78,  60},
            {"S005", MATERIAL_NAMES[4], 3000, 0.90, 120},
        };

        SampleJsonRepository repo(SAMPLE_DIR);
        std::cout << "\n[Setup] sampledata/ 에 기본 시료 없음"
                     " — DummyDataGenerator 로직으로 5개 자동 생성\n";
        std::cout << std::string(74, '-') << "\n";

        for (const auto& d : defaults) {
            if (!repo.exists(d.id)) {
                Sample s{ d.id, d.name, d.avgProductionTime, d.yield, d.stock };
                if (repo.add(s)) {
                    std::cout << "  [자동생성] ";
                    printSample(s);
                }
            }
        }

        std::cout << std::string(74, '-') << "\n\n";
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// READ — 사전 생성된 시료 5개 조회
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Read_PreExisting_AllLoaded) {
    auto all = repo.getAll();

    std::cout << "\n[READ] 전체 목록 (" << all.size() << "개)\n";
    std::cout << std::string(74, '-') << "\n";
    for (const auto& s : all)
        printSample(s);
    std::cout << std::string(74, '-') << "\n";

    EXPECT_GE(all.size(), PRE_IDS.size())
        << "sampledata/ 에 S001-S005 파일이 있어야 합니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Read_PreExisting_EachIdFound) {
    std::cout << "\n[READ] 사전 시료 ID별 조회\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& id : PRE_IDS) {
        auto s = repo.findById(id);
        ASSERT_TRUE(s.has_value()) << "ID 조회 실패: " << id;
        printSample(*s);
        EXPECT_EQ(s->id, id);
        EXPECT_FALSE(s->name.empty());
        EXPECT_GT(s->avgProductionTime, 0);
        EXPECT_GT(s->yield, 0.0);
        EXPECT_LE(s->yield, 1.0);
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Read_PreExisting_FindByName) {
    auto result = repo.findByName("웨이퍼");
    std::cout << "\n[READ] findByName(\"웨이퍼\") → " << result.size() << "건\n";
    EXPECT_GE(result.size(), PRE_IDS.size());
}

// ═══════════════════════════════════════════════════════════════════════════
// UPDATE — 사전 생성된 시료 5개 수정
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Update_PreExisting_StockDelta) {
    std::cout << "\n[UPDATE] 사전 시료 재고 +10\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& id : PRE_IDS) {
        auto before = repo.findById(id);
        ASSERT_TRUE(before.has_value());
        int prevStock = before->stock;

        EXPECT_TRUE(repo.updateStock(id, 10));

        SampleJsonRepository reloaded(SAMPLE_DIR);
        auto after = reloaded.findById(id);
        ASSERT_TRUE(after.has_value());

        printDiff(*before, *after);

        EXPECT_EQ(after->stock, prevStock + 10) << id << " 재고 갱신 불일치";
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Update_PreExisting_ProductionTimeChanged) {
    std::cout << "\n[UPDATE] 사전 시료 생산시간 +100초\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& id : PRE_IDS) {
        auto orig = repo.findById(id);
        ASSERT_TRUE(orig.has_value());

        Sample updated = *orig;
        updated.avgProductionTime = orig->avgProductionTime + 100;
        EXPECT_TRUE(repo.update(updated));

        SampleJsonRepository reloaded(SAMPLE_DIR);
        auto result = reloaded.findById(id);
        ASSERT_TRUE(result.has_value());

        printDiff(*orig, *result);

        EXPECT_EQ(result->avgProductionTime, orig->avgProductionTime + 100)
            << id << " 생산시간 갱신 불일치";
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Read_PreExisting_AfterUpdate_Confirm) {
    std::cout << "\n[READ] 수정 후 사전 시료 현재 상태\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& id : PRE_IDS) {
        auto s = repo.findById(id);
        ASSERT_TRUE(s.has_value()) << id << " 수정 후 조회 실패";
        printSample(*s);
    }

    std::cout << std::string(74, '-') << "\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// CREATE — 신규 시료 T001-T005 생성
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Create_NewSamples_AddSucceeds) {
    int before = static_cast<int>(repo.getAll().size());

    std::cout << "\n[CREATE] 신규 시료 T001-T005 추가\n";
    std::cout << std::string(74, '-') << "\n";

    for (int i = 0; i < 5; ++i) {
        Sample s;
        s.id                = NEW_IDS[i];
        s.name              = "테스트 시료 " + std::to_string(i + 1);
        s.avgProductionTime = 600 + i * 60;
        s.yield             = 0.80 + i * 0.02;
        s.stock             = 50  + i * 10;
        EXPECT_TRUE(repo.add(s)) << "추가 실패: " << s.id;
        printSample(s);
    }

    std::cout << std::string(74, '-') << "\n";
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
    std::cout << "\n[READ] 신규 시료 T001-T005 조회\n";
    std::cout << std::string(74, '-') << "\n";

    for (int i = 0; i < 5; ++i) {
        auto s = repo.findById(NEW_IDS[i]);
        ASSERT_TRUE(s.has_value()) << "조회 실패: " << NEW_IDS[i];
        printSample(*s);
        EXPECT_EQ(s->id,   NEW_IDS[i]);
        EXPECT_EQ(s->name, "테스트 시료 " + std::to_string(i + 1));
        EXPECT_EQ(s->avgProductionTime, 600 + i * 60);
        EXPECT_DOUBLE_EQ(s->yield, 0.80 + i * 0.02);
        EXPECT_EQ(s->stock, 50 + i * 10);
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Read_NewSamples_AllExist) {
    for (const auto& id : NEW_IDS)
        EXPECT_TRUE(repo.exists(id)) << id << " 존재해야 합니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Read_NewSamples_FindByName) {
    auto result = repo.findByName("테스트 시료");
    std::cout << "\n[READ] findByName(\"테스트 시료\") → " << result.size() << "건\n";
    EXPECT_EQ(result.size(), NEW_IDS.size());
}

// ═══════════════════════════════════════════════════════════════════════════
// UPDATE — 신규 생성된 T001-T005 수정
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Update_NewSamples_StockIncreased) {
    std::cout << "\n[UPDATE] 신규 시료 재고 +100\n";
    std::cout << std::string(74, '-') << "\n";

    for (int i = 0; i < 5; ++i) {
        auto before = repo.findById(NEW_IDS[i]);
        ASSERT_TRUE(before.has_value());

        EXPECT_TRUE(repo.updateStock(NEW_IDS[i], 100));

        SampleJsonRepository reloaded(SAMPLE_DIR);
        auto after = reloaded.findById(NEW_IDS[i]);
        ASSERT_TRUE(after.has_value());

        printDiff(*before, *after);

        EXPECT_EQ(after->stock, 50 + i * 10 + 100) << NEW_IDS[i] << " 재고 갱신 불일치";
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Update_NewSamples_FieldsPersisted) {
    std::cout << "\n[UPDATE] 신규 시료 생산시간·수율 변경 (→ 9999초 / 0.95)\n";
    std::cout << std::string(74, '-') << "\n";

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

        printDiff(*orig, *result);

        EXPECT_EQ(result->avgProductionTime, 9999);
        EXPECT_DOUBLE_EQ(result->yield, 0.95);
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Read_NewSamples_AfterUpdate_Confirm) {
    std::cout << "\n[READ] 수정 후 신규 시료 현재 상태\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& id : NEW_IDS) {
        auto s = repo.findById(id);
        ASSERT_TRUE(s.has_value()) << id << " 수정 후 조회 실패";
        printSample(*s);
    }

    std::cout << std::string(74, '-') << "\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// DELETE — 신규 생성된 T001-T005 삭제
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC_SampleTest, TC_Sample_Delete_NewSamples_RemoveSucceeds) {
    std::cout << "\n[DELETE] 신규 시료 T001-T005 삭제\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& id : NEW_IDS) {
        EXPECT_TRUE(repo.remove(id)) << "삭제 실패: " << id;
        std::cout << "  [삭제] " << id << "\n";
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Delete_NewSamples_NoLongerExists) {
    for (const auto& id : NEW_IDS)
        EXPECT_FALSE(repo.exists(id)) << id << " 는 삭제 후 존재하면 안됩니다.";
}

TEST_F(TC_SampleTest, TC_Sample_Delete_PreExisting_Intact) {
    std::cout << "\n[READ] 삭제 후 사전 시료 잔존 확인\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& id : PRE_IDS) {
        EXPECT_TRUE(repo.exists(id)) << id << " 사전 시료가 삭제되었습니다.";
        auto s = repo.findById(id);
        if (s.has_value()) printSample(*s);
    }

    std::cout << std::string(74, '-') << "\n";
}

TEST_F(TC_SampleTest, TC_Sample_Delete_NonExisting_ReturnsFalse) {
    EXPECT_FALSE(repo.remove("NONE"));
}
