#include "StockInfo.h"

std::string stockStatusToString(StockStatus s) {
    switch (s) {
    case StockStatus::SURPLUS:  return "여유";
    case StockStatus::SHORTAGE: return "부족";
    case StockStatus::DEPLETED: return "고갈";
    default:                    return "?";
    }
}
