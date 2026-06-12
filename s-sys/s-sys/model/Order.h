#pragma once
#include <string>

enum class OrderStatus {
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASE
};

std::string orderStatusToString(OrderStatus status);

struct Order {
    int         id = 0;
    std::string sampleId;
    std::string customerName;
    int         quantity = 0;
    OrderStatus status = OrderStatus::RESERVED;
};
