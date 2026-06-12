#define __GTEST__ (1)

#ifdef __GTEST__

// ─── GTest 수행 환경 ──────────────────────────────────────────────────────────
#include <gtest/gtest.h>
#include <windows.h>

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#else

// ─── 일반 실행 환경 (MVC 메인 메뉴) ─────────────────────────────────────────────
#include <iostream>
#include <string>
#include <windows.h>

#include "model/SampleRepository.h"
#include "model/OrderRepository.h"
#include "model/ProductionQueue.h"
#include "model/AutoProductionService.h"

#include "controller/SampleController.h"
#include "controller/OrderController.h"
#include "controller/MonitorController.h"
#include "controller/ProductionController.h"
#include "controller/ShipmentController.h"

#include "view/MainView.h"
#include "view/SampleView.h"
#include "view/OrderView.h"
#include "view/MonitorView.h"
#include "view/ProductionView.h"
#include "view/ShipmentView.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SampleRepository       sampleRepo;
    OrderRepository        orderRepo;
    ProductionQueue        productionQueue;
    AutoProductionService  autoProduction(sampleRepo);

    SampleController     sampleCtrl(sampleRepo);
    OrderController      orderCtrl(orderRepo, sampleRepo, productionQueue);
    MonitorController    monitorCtrl(orderRepo, sampleRepo);
    ProductionController prodCtrl(productionQueue, orderRepo, sampleRepo);
    ShipmentController   shipCtrl(orderRepo);

    MainView       mainView;
    SampleView     sampleView;
    OrderView      orderView;
    MonitorView    monitorView;
    ProductionView prodView;
    ShipmentView   shipView;

    bool running = true;
    while (running) {
        // ① 재고 자동 생산 (타이머)
        for (const auto& ev : autoProduction.tick())
            std::cout << "[자동생산] " << ev.sampleId << " " << ev.sampleName
                      << "  +" << ev.unitsProduced << "개\n";

        // ② 생산 라인 큐 자동 완료 (totalTime 경과 시)
        if (auto done = prodCtrl.tickProduction()) {
            std::cout << "[생산완료] 주문 #" << done->orderId
                      << "  시료 " << done->sampleId
                      << "  +" << done->actualQty << "개 → CONFIRMED\n";
            // 대기 큐에 다음 작업이 있으면 자동 시작 알림
            if (auto next = prodCtrl.getCurrentProduction())
                std::cout << "[생산시작] 주문 #" << next->orderId
                          << "  시료 " << next->sampleId
                          << "  생산 시작 (총 " << next->totalTime << "초)\n";
        }

        mainView.displayMenu();
        switch (mainView.getChoice()) {

        case MainMenuChoice::SAMPLE_MANAGEMENT: {
            bool inMenu = true;
            while (inMenu) {
                sampleView.displayMenu();
                switch (sampleView.getChoice()) {
                case SampleMenuChoice::REGISTER: {
                    auto s  = sampleView.inputSampleData();
                    bool ok = sampleCtrl.registerSample(s.id, s.name,
                                                        s.avgProductionTime, s.yield);
                    sampleView.displayResult(ok, ok ? "등록 완료" : "등록 실패 (중복 ID 또는 잘못된 값)");
                    break;
                }
                case SampleMenuChoice::LIST:
                    sampleView.displaySamples(sampleCtrl.getAllSamples());
                    break;
                case SampleMenuChoice::SEARCH: {
                    auto name = sampleView.inputSearchName();
                    sampleView.displaySamples(sampleCtrl.searchByName(name));
                    break;
                }
                case SampleMenuChoice::BACK:
                    inMenu = false;
                    break;
                }
            }
            break;
        }

        case MainMenuChoice::ORDER_MANAGEMENT: {
            bool inMenu = true;
            while (inMenu) {
                orderView.displayMenu();
                switch (orderView.getChoice()) {
                case OrderMenuChoice::PLACE: {
                    auto input = orderView.inputOrderData();
                    int  id    = orderCtrl.placeOrder(input.sampleId,
                                                       input.customerName, input.quantity);
                    orderView.displayResult(id > 0,
                        id > 0 ? "주문 접수 완료 (주문 ID: " + std::to_string(id) + ")"
                               : "주문 실패 (미등록 시료 또는 잘못된 수량)");
                    break;
                }
                case OrderMenuChoice::APPROVE_REJECT: {
                    orderView.displayOrders(orderCtrl.getReservedOrders());
                    int orderId = orderView.inputOrderId();
                    switch (orderView.getApproveRejectChoice()) {
                    case ApproveRejectChoice::APPROVE: {
                        // 승인 전 생산 라인 상태 스냅샷
                        bool hadRunning = prodCtrl.getCurrentProduction().has_value();
                        bool ok = orderCtrl.approveOrder(orderId);
                        if (ok) {
                            auto currJob = prodCtrl.getCurrentProduction();
                            int  waiting = static_cast<int>(prodCtrl.getWaitingJobs().size());
                            if (!currJob) {
                                // 재고 충분 → 즉시 CONFIRMED
                                orderView.displayResult(ok, "주문 승인 완료 (재고 충분 → CONFIRMED)");
                            } else if (hadRunning) {
                                // 이미 생산 중 → 대기 큐에 추가
                                orderView.displayResult(ok,
                                    "주문 승인 완료 → 대기 큐 추가 (현재 대기 " +
                                    std::to_string(waiting) + "건, 앞 작업 완료 후 생산 시작)");
                            } else {
                                // 생산 라인 비어 있었음 → 즉시 생산 투입
                                orderView.displayResult(ok, "주문 승인 완료 → 생산 라인 즉시 투입");
                            }
                        } else {
                            orderView.displayResult(ok, "승인 실패");
                        }
                        break;
                    }
                    case ApproveRejectChoice::REJECT: {
                        bool ok = orderCtrl.rejectOrder(orderId);
                        orderView.displayResult(ok, ok ? "주문 거절 완료" : "거절 실패");
                        break;
                    }
                    case ApproveRejectChoice::BACK:
                        break;
                    }
                    break;
                }
                case OrderMenuChoice::STATUS:
                    orderView.displayAllOrders(orderCtrl.getAllOrders());
                    break;
                case OrderMenuChoice::BACK:
                    inMenu = false;
                    break;
                }
            }
            break;
        }

        case MainMenuChoice::MONITORING: {
            for (auto status : { OrderStatus::RESERVED, OrderStatus::CONFIRMED,
                                  OrderStatus::PRODUCING, OrderStatus::RELEASE }) {
                monitorView.displayOrdersByStatus(
                    monitorCtrl.getOrdersByStatus(status), status);
            }
            monitorView.displayStockInfo(monitorCtrl.getStockInfo());
            break;
        }

        case MainMenuChoice::SHIPMENT: {
            shipView.displayConfirmedOrders(shipCtrl.getConfirmedOrders());
            int orderId = shipView.inputOrderId();
            if (orderId > 0) {
                bool ok = shipCtrl.release(orderId);
                shipView.displayResult(ok, ok ? "출고 완료 (RELEASE)" : "출고 실패");
            }
            break;
        }

        case MainMenuChoice::PRODUCTION_LINE: {
            bool inMenu = true;
            while (inMenu) {
                // 생산 라인 진입 시에도 자동 완료 체크
                if (auto done = prodCtrl.tickProduction()) {
                    std::cout << "[생산완료] 주문 #" << done->orderId
                              << "  시료 " << done->sampleId
                              << "  +" << done->actualQty << "개 → CONFIRMED\n";
                    if (auto next = prodCtrl.getCurrentProduction())
                        std::cout << "[생산시작] 주문 #" << next->orderId
                                  << "  시료 " << next->sampleId
                                  << "  생산 시작 (총 " << next->totalTime << "초)\n";
                }

                prodView.displayCurrentProduction(prodCtrl.getCurrentProductionProgress());
                prodView.displayWaitingQueue(prodCtrl.getWaitingJobs());
                switch (prodView.getChoice()) {
                case ProductionMenuChoice::COMPLETE: {
                    bool ok = prodCtrl.completeCurrentProduction();
                    prodView.displayResult(ok, ok ? "생산 완료 처리 (CONFIRMED 전환)" : "생산 중인 작업 없음");
                    break;
                }
                case ProductionMenuChoice::BACK:
                    inMenu = false;
                    break;
                }
            }
            break;
        }

        case MainMenuChoice::EXIT:
            std::cout << "\n시스템을 종료합니다.\n";
            running = false;
            break;
        }
    }
    return 0;
}

#endif // __GTEST__
