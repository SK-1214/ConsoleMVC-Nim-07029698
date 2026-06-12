#include "data/OrderJsonRepository.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <windows.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

static std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) return {};
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring ws(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, ws.data(), wlen);
    return ws;
}

static OrderStatus statusFromString(const std::string& s) {
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASE")   return OrderStatus::RELEASE;
    return OrderStatus::RESERVED;
}

static Order orderFromJson(const json& j) {
    Order o;
    o.id           = j.at("id").get<int>();
    o.sampleId     = j.at("sampleId").get<std::string>();
    o.customerName = j.at("customerName").get<std::string>();
    o.quantity     = j.at("quantity").get<int>();
    o.status       = statusFromString(j.at("status").get<std::string>());
    return o;
}

static json orderToJson(const Order& o) {
    return json{
        {"id",           o.id},
        {"sampleId",     o.sampleId},
        {"customerName", o.customerName},
        {"quantity",     o.quantity},
        {"status",       orderStatusToString(o.status)}
    };
}

// ─────────────────────────────────────────────────────────────────────────────

OrderJsonRepository::OrderJsonRepository(const std::string& dir) : dir_(dir) {
    load();
}

std::string OrderJsonRepository::filePath() const {
    return dir_ + "/orders.json";
}

void OrderJsonRepository::load() {
    orders_.clear();
    nextId_ = 1;

    fs::path fp(toWide(filePath()));
    if (!fs::exists(fp)) return;

    std::ifstream ifs(fp.wstring().c_str());
    if (!ifs.is_open()) return;

    try {
        json root = json::parse(ifs);
        nextId_ = root.value("nextId", 1);
        for (const auto& item : root.value("orders", json::array()))
            orders_.push_back(orderFromJson(item));
    } catch (...) {}
}

bool OrderJsonRepository::save() const {
    fs::create_directories(fs::path(toWide(dir_)));
    std::ofstream ofs(toWide(filePath()).c_str());
    if (!ofs.is_open()) return false;

    json root;
    root["nextId"]  = nextId_;
    root["orders"]  = json::array();
    for (const auto& o : orders_)
        root["orders"].push_back(orderToJson(o));
    ofs << root.dump(2);
    return true;
}

int OrderJsonRepository::add(const Order& order) {
    Order o    = order;
    o.id       = nextId_++;
    orders_.push_back(o);
    save();
    return o.id;
}

std::optional<Order> OrderJsonRepository::findById(int id) const {
    for (const auto& o : orders_)
        if (o.id == id) return o;
    return std::nullopt;
}

std::vector<Order> OrderJsonRepository::getAll() const {
    return orders_;
}

std::vector<Order> OrderJsonRepository::getByStatus(OrderStatus status) const {
    std::vector<Order> result;
    for (const auto& o : orders_)
        if (o.status == status) result.push_back(o);
    return result;
}

bool OrderJsonRepository::updateStatus(int id, OrderStatus status) {
    for (auto& o : orders_) {
        if (o.id == id) {
            o.status = status;
            save();
            return true;
        }
    }
    return false;
}
