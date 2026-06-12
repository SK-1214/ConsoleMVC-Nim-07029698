#pragma once
#include <string>
#include <vector>
#include <optional>
#include "model/Order.h"

/*
 * producedata/orders.json 단일 파일 기반 Order 저장소
 * { "nextId": N, "orders": [...] }
 */
class OrderJsonRepository {
public:
    explicit OrderJsonRepository(const std::string& dir = "producedata");

    int                  add(const Order& order);
    std::optional<Order> findById(int id) const;
    std::vector<Order>   getAll() const;
    std::vector<Order>   getByStatus(OrderStatus status) const;
    bool                 updateStatus(int id, OrderStatus status);
    int                  getNextId() const { return nextId_; }

private:
    std::string        dir_;
    std::vector<Order> orders_;
    int                nextId_ = 1;

    std::string filePath() const;
    void        load();
    bool        save() const;
};
