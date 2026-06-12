# TC Generation Skill

지정한 기능에 대한 GoogleTest 기반 TC(Test Case) 파일을 생성한다.

## 호출 방법

```
/tc_generation  {대상 기능 설명}에 대한 TC를 만들어줘
```

## 동작 절차

1. 사용자에게 **"헤더명은 뭐야?"** 라고 질문한다.
2. 입력받은 헤더명을 기반으로 아래 규칙에 따라 파일을 생성한다.

## 파일 명명 규칙

```
testcase/TC_{헤더이름}_{기능구분}.cpp
```

- `{헤더이름}` : 사용자가 답한 헤더명 (예: SampleOrder → TC_SampleOrder_xxx)
- `{기능구분}` : CREATE / READ / UPDATE / DELETE / FLOW 등 TC 성격에 맞게 분류

예시:
```
testcase/TC_SampleOrder_Create.cpp
testcase/TC_SampleOrder_Read.cpp
testcase/TC_SampleOrder_Update.cpp
testcase/TC_SampleOrder_Delete.cpp
```

## TC 파일 생성 규칙

### 파일 위치
- `s-sys/s-sys/testcase/` 폴더에 생성

### 파일 등록
- `s-sys.vcxproj` 의 `<ClCompile>` 항목에 추가
- `s-sys.vcxproj.filters` 의 `소스 파일\testcase` 필터에 추가

### TC 코드 형식 (GoogleTest)

```cpp
#include <gtest/gtest.h>
#include "{헤더이름}.h"   // 또는 관련 헤더

// ─── 픽스처 ───────────────────────────────────────────────────────────────────
class TC_{헤더이름}Test : public ::testing::Test {
protected:
    void SetUp() override { /* 초기화 */ }
    void TearDown() override { /* 정리 */ }
};

// ─── CREATE ──────────────────────────────────────────────────────────────────
TEST_F(TC_{헤더이름}Test, Create_유효한입력_성공) { ... }
TEST_F(TC_{헤더이름}Test, Create_중복ID_실패)    { ... }

// ─── READ ────────────────────────────────────────────────────────────────────
TEST_F(TC_{헤더이름}Test, Read_존재하는ID_반환)  { ... }
TEST_F(TC_{헤더이름}Test, Read_없는ID_nullopt)   { ... }

// ... UPDATE / DELETE / FLOW 등 추가
```

### TC 이름 규칙
```
{동작}_{조건}_{기대결과}
예) Create_유효한입력_성공
    Read_존재하는ID_반환
    Delete_없는ID_실패
```

## 참고
- 기존 TC 예시: `s-sys/s-sys/data/SampleJsonRepository_test.cpp`
- PRD 기능 명세: `PRD.md`
- Reviewer Agent 규칙: `agents/reviewer.md`
