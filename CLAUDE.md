# CLAUDE.md — 반도체 시료 생산 주문 관리 시스템

## 프로젝트 개요

Visual Studio, **C++ 콘솔 기반** 프로그램.  
가상의 반도체 회사 S-Semi의 시료 생산·주문·출고를 관리하는 시스템이다.

- 솔루션 파일: `s-sys/s-sys.slnx`
- 메인 프로젝트: `s-sys/s-sys/s-sys.vcxproj`
- 아키텍처: **MVC (Model-View-Controller)**

전체 기능 명세는 **[PRD.md](PRD.md)** 를 참조한다.

---

## 아키텍처: MVC

### 레이어 정의

| 레이어 | 책임 | 파일 예시 |
|--------|------|-----------|
| **Model** | 데이터 구조 및 비즈니스 로직. 시료·주문·생산 큐의 상태 관리 | `Sample.h/cpp`, `Order.h/cpp`, `ProductionQueue.h/cpp` |
| **View** | 콘솔 입출력 전담. 메뉴 출력, 사용자 입력 수신, 결과 표시 | `SampleView.h/cpp`, `OrderView.h/cpp`, `MonitorView.h/cpp` |
| **Controller** | Model과 View를 연결. 사용자 입력을 해석하여 Model을 조작하고 View에 전달 | `SampleController.h/cpp`, `OrderController.h/cpp` |

### 의존 방향

```
View  ──→  Controller  ──→  Model
```

- View는 Controller만 호출하고 Model을 직접 참조하지 않는다.
- Model은 View와 Controller를 알지 못한다.
- Controller가 Model의 결과를 받아 View에 전달한다.

### 파일 구조

```
s-sys/s-sys/
├── model/
│   ├── Sample.h / Sample.cpp
│   ├── Order.h / Order.cpp
│   └── ProductionQueue.h / ProductionQueue.cpp
├── view/
│   ├── SampleView.h / SampleView.cpp
│   ├── OrderView.h / OrderView.cpp
│   ├── MonitorView.h / MonitorView.cpp
│   └── ProductionView.h / ProductionView.cpp
├── controller/
│   ├── SampleController.h / SampleController.cpp
│   ├── OrderController.h / OrderController.cpp
│   ├── MonitorController.h / MonitorController.cpp
│   └── ProductionController.h / ProductionController.cpp
├── *_test.cpp          ← Reviewer가 작성하는 TC
└── main.cpp
```

### 테스트 대상

- **Model** 레이어를 우선 단위 테스트한다.
- Controller는 Model을 Mock으로 대체하여 테스트한다.
- View는 콘솔 I/O 특성상 TC에서 제외하거나 별도 Mock Stream으로 처리한다.

---

## 테스트 환경

- 프레임워크: **GoogleTest / GoogleMock**
- 설치 방식: NuGet (`Microsoft.googletest.v140.windesktop.msvcstl.static.rt-static.1.8.1.8`)
- 헤더 위치: `s-sys/packages/.../build/native/include/gtest/gtest.h`
- 테스트 파일 패턴: `s-sys/s-sys/*_test.cpp`

---

## Agent 역할 분리

이 프로젝트는 두 개의 전담 Agent가 협력하여 개발한다.

| Agent | 역할 | 수정 가능 파일 | 수정 불가 파일 |
|-------|------|---------------|---------------|
| **Dev** | 기능 구현 | `*.h`, `*.cpp` (테스트 제외) | `*_test.cpp` |
| **Reviewer** | TC 작성 | `*_test.cpp` | 구현 코드 전체 |

각 Agent의 상세 명세:

- Dev Agent → [`agents/dev.md`](agents/dev.md)
- Reviewer Agent → [`agents/reviewer.md`](agents/reviewer.md)

### 핵심 규칙

- **Dev**는 테스트 코드를 절대 수정하지 않는다. TC가 FAIL이면 구현 코드를 수정한다.
- **Reviewer**는 구현 코드를 절대 수정하지 않는다. TC가 FAIL이더라도 구현 코드에 손대지 않는다.
- TC의 assertion·기대값은 Reviewer만 결정하며, PASS를 위해 기대값을 완화하지 않는다.

---

## 개발 흐름

```
1. Reviewer → PRD.md 기반으로 *_test.cpp 작성
2. Dev      → TC 인터페이스를 읽고 헤더(.h) 설계
3. Dev      → 소스(.cpp) 구현
4. 빌드 & 테스트 실행
5. TC FAIL  → Dev가 구현 코드만 수정하여 재시도
6. 모든 TC PASS → 완료
```

---

## 주문 상태 전이 요약

```
RESERVED ──[승인, 재고 충분]──→ CONFIRMED ──[출고]──→ RELEASE
RESERVED ──[승인, 재고 부족]──→ PRODUCING ──[생산 완료]──→ CONFIRMED ──[출고]──→ RELEASE
RESERVED ──[거절]──────────→ REJECTED  (모니터링 제외)
```

---

## 주요 계산 공식

```cpp
// 생산량 계산 (생산 라인)
int actualQty = (int)ceil(shortage / (yield * 0.9));
int totalTime  = avgProductionTime * actualQty;
```

---

## 참조 문서

| 문서 | 경로 |
|------|------|
| 기능 명세 (PRD) | [`PRD.md`](PRD.md) |
| Dev Agent 명세 | [`agents/dev.md`](agents/dev.md) |
| Reviewer Agent 명세 | [`agents/reviewer.md`](agents/reviewer.md) |
