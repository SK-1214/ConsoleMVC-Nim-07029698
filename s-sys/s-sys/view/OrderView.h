#pragma once
#include <string>
#include <vector>
#include "model/Order.h"

enum class OrderMenuChoice      { PLACE = 1, APPROVE_REJECT, BACK = 0 };
enum class ApproveRejectChoice  { APPROVE = 1, REJECT, BACK = 0 };

struct OrderInput {
    std::string sampleId;
    std::string customerName;
    int quantity = 0;
};

class OrderView {
public:
    void displayMenu() const;
    OrderMenuChoice getChoice() const;
    OrderInput inputOrderData() const;
    int inputOrderId() const;
    ApproveRejectChoice getApproveRejectChoice() const;
    void displayOrders(const std::vector<Order>& orders) const;
    void displayResult(bool success, const std::string& msg) const;
};
