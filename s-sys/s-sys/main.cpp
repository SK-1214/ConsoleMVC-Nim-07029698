#include <iostream>
#include <string>

#include "model/SampleRepository.h"
#include "model/OrderRepository.h"
#include "model/ProductionQueue.h"

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
    // --- Model ---
    SampleRepository sampleRepo;
    OrderRepository  orderRepo;
    ProductionQueue  productionQueue;

    // --- Controller ---
    SampleController    sampleCtrl(sampleRepo);
    OrderController     orderCtrl(orderRepo, sampleRepo, productionQueue);
    MonitorController   monitorCtrl(orderRepo, sampleRepo);
    ProductionController prodCtrl(productionQueue, orderRepo, sampleRepo);
    ShipmentController  shipCtrl(orderRepo);

    // --- View ---
    MainView       mainView;
    SampleView     sampleView;
    OrderView      orderView;
    MonitorView    monitorView;
    ProductionView prodView;
    ShipmentView   shipView;

    bool running = true;
    while (running) {
        mainView.displayMenu();
        switch (mainView.getChoice()) {

        // ── 1. 시료 관리 ─────────────────────────────────────────
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

        // ── 2. 주문 관리 ─────────────────────────────────────────
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
                        bool ok = orderCtrl.approveOrder(orderId);
                        orderView.displayResult(ok, ok ? "주문 승인 완료" : "승인 실패");
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
                case OrderMenuChoice::BACK:
                    inMenu = false;
                    break;
                }
            }
            break;
        }

        // ── 3. 모니터링 ──────────────────────────────────────────
        case MainMenuChoice::MONITORING: {
            for (auto status : { OrderStatus::RESERVED, OrderStatus::CONFIRMED,
                                  OrderStatus::PRODUCING, OrderStatus::RELEASE }) {
                monitorView.displayOrdersByStatus(
                    monitorCtrl.getOrdersByStatus(status), status);
            }
            monitorView.displayStockInfo(monitorCtrl.getStockInfo());
            break;
        }

        // ── 4. 출고 처리 ─────────────────────────────────────────
        case MainMenuChoice::SHIPMENT: {
            shipView.displayConfirmedOrders(shipCtrl.getConfirmedOrders());
            int orderId = shipView.inputOrderId();
            if (orderId > 0) {
                bool ok = shipCtrl.release(orderId);
                shipView.displayResult(ok, ok ? "출고 완료 (RELEASE)" : "출고 실패");
            }
            break;
        }

        // ── 5. 생산 라인 ─────────────────────────────────────────
        case MainMenuChoice::PRODUCTION_LINE: {
            bool inMenu = true;
            while (inMenu) {
                prodView.displayCurrentProduction(prodCtrl.getCurrentProduction());
                prodView.displayWaitingQueue(prodCtrl.getWaitingJobs());
                switch (prodView.getChoice()) {
                case ProductionMenuChoice::COMPLETE: {
                    bool ok = prodCtrl.completeCurrentProduction();
                    prodView.displayResult(ok, ok ? "생산 완료 (CONFIRMED 전환)" : "생산 중인 작업 없음");
                    break;
                }
                case ProductionMenuChoice::BACK:
                    inMenu = false;
                    break;
                }
            }
            break;
        }

        // ── 0. 종료 ──────────────────────────────────────────────
        case MainMenuChoice::EXIT:
            std::cout << "\n시스템을 종료합니다.\n";
            running = false;
            break;
        }
    }
    return 0;
}
